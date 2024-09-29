package server

import (
	"context"
	"errors"
	"fmt"
	"io"
	"net/http"
	"net/http/httptest"
	"strings"
	"sync"
	"testing"
	"time"

	pb "github.com/Milover/beholder/internal/server/proto"
	"github.com/coder/websocket"
	"github.com/google/uuid"
	"github.com/stretchr/testify/assert"
	"google.golang.org/protobuf/proto"
)

// For future reference, the recommended command to run the tests is:
//
//	go test -v -race -count=N

// errAcq is the acquisition error used by mockAcq.
var errAcq = errors.New("acquisition error")

// mockAcq implements the [AcquisitionStartStopper] interface.
type mockAcq struct {
	acquiring bool       // reports whether acquisition is running
	blobs     chan *Blob // channel to send blobs on
	errs      chan error // channel to send errors on
}

// acquire simulates a single acquisition.
// If acq.acquiring == true, it generates a [*Blob] with a new UUID and
// [Blob.Source] set to 'test'.
// The generated [*Blob] is sent on [mockAck.blobs] and returned.
func (acq *mockAcq) acquire() *Blob {
	if acq.acquiring {
		blob := &Blob{UUID: uuid.Must(uuid.NewV7()), Source: "test"}
		acq.blobs <- blob
		return blob
	}
	return nil
}

// acquireErr simulates an (irrecoverable) acquisition error.
// If acq.acquiring == ture, it sends an [errAcq] on acq.errs and
// stops acquisition.
func (acq *mockAcq) acquireErr() {
	if acq.acquiring {
		acq.errs <- errAcq
		acq.StopAcquisition()
	}
}

// assertAcquisitionStarted asserts that all requirements for acquisition-start
// are satisfied.
func (acq *mockAcq) assertAcquisitionStarted() error {
	switch {
	case acq.blobs == nil:
		return errors.New("nil blobs channel")
	case acq.errs == nil:
		return errors.New("nil errs channel")
	case !acq.acquiring:
		return errors.New("acquiring != true")
	default:
		return nil
	}
}

// assertAcquisitionStopped asserts that all requirements for acquisition-stop
// are satisfied.
//
// WARNING: assertAcquisitionStopped reads from both acq.blobs and acq.errs
// to check if they are closed.
func (acq *mockAcq) assertAcquisitionStopped() error {
	_, blobsOK := <-acq.blobs
	_, errsOK := <-acq.errs
	switch {
	case blobsOK:
		return errors.New("blobs channel not closed")
	case errsOK:
		return errors.New("errs channel not closed")
	case acq.acquiring:
		return errors.New("acquiring != false")
	default:
		return nil
	}
}

// IsAcquiring reports whether the state of acq.acquiring.
func (acq *mockAcq) IsAcquiring() bool {
	return acq.acquiring
}

// StartAcquisition sets up and returns the comms channels and
// sets acq.acquiring to true.
func (acq *mockAcq) StartAcquisition(blobsSize int) (<-chan *Blob, <-chan error) {
	if acq.IsAcquiring() {
		return acq.blobs, acq.errs
	}
	acq.blobs = make(chan *Blob, blobsSize)
	acq.errs = make(chan error, 1)
	acq.acquiring = true
	return acq.blobs, acq.errs
}

// StopAcquisition closes the comms channels and sets acq.acquiring to false.
func (acq *mockAcq) StopAcquisition() {
	if acq.acquiring {
		close(acq.blobs)
		close(acq.errs)
		acq.acquiring = false
	}
}

// sendRequest sends an HTTP request to a handler and
// returns the response status code.
func sendRequest(method, route string, handler http.HandlerFunc) int {
	req := httptest.NewRequest(method, route, nil)
	w := httptest.NewRecorder()

	handler(w, req)
	resp := w.Result()
	defer resp.Body.Close()

	_, _ = io.ReadAll(resp.Body)
	return resp.StatusCode
}

// routeTest is a structure used for testing [AcquisitionServer] route handlers.
type routeTest struct {
	// route is the endpoint being tested.
	route string
	// method is the HTTP request method.
	method string
	// expStatus is the expected HTTP status code.
	expStatus int
	// handler is the handler being tested.
	handler func(*AcquisitionServer, http.ResponseWriter, *http.Request)
	// pass generates the test-pass condition.
	pass func(*AcquisitionServer, *mockAcq) error
	// setup is a function run before sending the request.
	setup func(*AcquisitionServer, *mockAcq) error
}

var routeTests = []routeTest{
	{
		route:     "/acquisition-start",
		method:    http.MethodPost,
		expStatus: http.StatusSeeOther,
		handler:   (*AcquisitionServer).acquisitionStartHandler,
		pass: func(as *AcquisitionServer, m *mockAcq) error {
			return m.assertAcquisitionStarted()
		},
	},
	{
		route:     "/acquisition-stop",
		method:    http.MethodPost,
		expStatus: http.StatusSeeOther,
		handler:   (*AcquisitionServer).acquisitionStopHandler,
		pass: func(_ *AcquisitionServer, m *mockAcq) error {
			return m.assertAcquisitionStopped()
		},
		setup: func(as *AcquisitionServer, m *mockAcq) error {
			// we have to start acquisition first for the test to make sense
			statusCode := sendRequest(http.MethodPost, "/acquisition-start",
				as.acquisitionStartHandler)
			if statusCode != http.StatusSeeOther {
				return fmt.Errorf("expected status 'See Other', but got: %v", statusCode)
			}
			return nil
		},
	},
	{
		route:     "/stream",
		method:    http.MethodGet,
		expStatus: http.StatusUpgradeRequired,
		handler:   (*AcquisitionServer).streamHandler,
		pass: func(as *AcquisitionServer, _ *mockAcq) error {
			// wait a bit for the connection to get removed
			time.Sleep(time.Millisecond * 100)
			as.connsMu.Lock()
			defer as.connsMu.Unlock()
			if as.conns.Len() != 0 {
				return fmt.Errorf("expected no connections, but got: %v", as.conns.Len())
			}
			return nil
		},
	},
}

// TestCustomHandlers tests the custom HTTP handlers of [AcquisitionServer].
//
// TODO: we should also test the file server handlers at some point, they
// seem like they could be a security risk.
func TestCustomHandlers(t *testing.T) {
	t.Parallel()
	for _, tt := range routeTests {
		t.Run(strings.TrimPrefix(tt.route, "/"), func(t *testing.T) {
			t.Parallel()
			assert := assert.New(t)

			// set up the servers
			mock := &mockAcq{}
			as, err := NewAcquisitionServer(mock)
			assert.Nil(err, err)

			// run setup
			if tt.setup != nil {
				err := tt.setup(as, mock)
				assert.Nil(err, err)
			}

			// test the route
			statusCode := sendRequest(tt.method, tt.route,
				func(w http.ResponseWriter, r *http.Request) {
					tt.handler(as, w, r)
				})
			assert.Equal(tt.expStatus, statusCode)

			err = tt.pass(as, mock)
			assert.Nil(err, err)
		})
	}
}

// client wraps a WebSocket connection.
type client struct {
	url    string          // test server URL
	httpCl *http.Client    // test server client
	c      *websocket.Conn // raw WebSocket connection
}

// newClient creates a new client with a WebSocket connection established
// at url + endpoint.
func newClient(ctx context.Context, url, endpoint string, cl *http.Client) (*client, error) {
	c, _, err := websocket.Dial(ctx, url+endpoint, nil)
	if err != nil {
		return nil, err
	}
	return &client{url: url, httpCl: cl, c: c}, nil
}

// Close closes the client WebSocket connection.
func (cl *client) Close() error {
	return cl.c.Close(websocket.StatusNormalClosure, "")
}

// nextMessage reads a message from the clients WebSocket connection and
// unmarshalls it from a protobuf.
func (cl *client) nextMessage() (proto.Message, error) {
	typ, b, err := cl.c.Read(context.Background())
	if err != nil {
		return nil, err
	}
	if typ != websocket.MessageBinary {
		cl.c.Close(websocket.StatusUnsupportedData, "expected binary message")
		return nil, fmt.Errorf("expected binary message, but got: %v", typ)
	}
	// unmarshall the message
	msg := &pb.MessageWrapper{}
	if err := proto.Unmarshal(b, msg); err != nil {
		return nil, err
	}
	return msg, nil
}

func (cl *client) doHTTPRequest(ctx context.Context, method, endpoint string, status int) error {
	reqCtx, reqCancel := context.WithTimeout(ctx, time.Second*5)
	defer reqCancel()

	req, err := http.NewRequestWithContext(reqCtx, method, cl.url+endpoint, nil)
	defer func() {
		if err != nil {
			err = fmt.Errorf("error at %q:", req.URL)
		}
	}()
	if err != nil {
		return err
	}

	resp, err := cl.httpCl.Do(req)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	_, err = io.ReadAll(resp.Body)
	if err != nil {
		return err
	}
	if resp.StatusCode != status {
		return fmt.Errorf("expected status %v, but got: %v", status, resp.StatusCode)
	}
	return nil
}

// TestImageMessage verifies that [AcquisitionServer] correctly starts and
// stops acquisition via client HTTP request, and correctly distributes acquired
// image-messages via WebSocket connections to all clients.
func TestMessageImage(t *testing.T) {
	rootPath = "." // mount "/" at current working directory
	t.Parallel()

	// This is a simple single client test.
	// The client requests an acquisition start, receives one message (blob)
	// and requests an acquisition stop.
	//
	// This test verifies that acquisition is started/stopped properly and
	// that the client receives the message without errors.
	t.Run("simple-msg-image", func(t *testing.T) {
		t.Parallel()
		assert := assert.New(t)

		// set up the servers
		mock := &mockAcq{}
		as, err := NewAcquisitionServer(mock)
		assert.Nil(err, err)

		srv := httptest.NewServer(as)
		defer srv.Close()

		// set up the client
		ctx, cancel := context.WithTimeout(context.Background(), time.Second*10)
		defer cancel()

		client, err := newClient(ctx, srv.URL, "/stream", srv.Client())
		assert.Nil(err, err)
		defer func() {
			err := client.Close()
			assert.Nil(err, fmt.Sprintf("client close error: %v", err))
		}()

		// start acquisition
		err = client.doHTTPRequest(ctx, http.MethodPost, "/acquisition-start", http.StatusOK)
		assert.Nil(err, err)
		err = mock.assertAcquisitionStarted()
		assert.Nil(err, err)

		// acquire (and send) blobs
		expMsg := NewMessage(mock.acquire())

		// read and check the message
		msg, err := client.nextMessage()
		assert.Nil(err, err)
		assert.True(proto.Equal(expMsg, msg))

		// stop acquisition
		err = client.doHTTPRequest(ctx, http.MethodPost, "/acquisition-stop", http.StatusOK)
		assert.Nil(err, err)
		err = mock.assertAcquisitionStopped()
		assert.Nil(err, err)
	})

	// This test is a real-worldish concurrent use scenario (eg. several
	// devs fucking around with a camera or two).
	// 10 clients are connected to the server and concurrently request
	// acquisition start/stop. Once acquisition starts, 24 messages (blobs)
	// are generated and sent to all clients concurrently.
	//
	// This test verifies that acquisition is started/stopped correctly,
	// even when requested concurrently, and that every message is received by
	// all clients without errors.
	//
	// The acquisition part seems complicated but it more-or-less simulates
	// a real-world scenario where:
	//	1. Blobs are being generated in a separate process/goroutine
	//     and are being continuously sent to the [AcquisitionServer].
	//	2. Clients concurrently request acquisition start/stop.
	//
	// We've made two simplifications in that:
	//	1. All clients establish connections before acquisition starts, so
	//     that we can require that all clients receive all generated messages,
	//     that is, any dropped messges are the result of an error on our end.
	//	2. Acquisition stop waits until all blobs have been generated, again
	//     to ensure all clients receive the expected number of messages.
	//	3. [AcquisitionServer.BufferSize] is set to the maximum number of
	//     messges simply because keeping it fixed and using a rate limiter
	//     makes the test flaky --- based on how the rate limiter is set up,
	//     how many different tests are run at the same time and the -count
	//     flag, the test may (or may not) fail with a 'connection too slow'.
	//
	// At this point there is no 'master' client which controls acquisition,
	// that is, any client can start or stop acquisition at any point.
	// This might become an issue at some point in the future, but
	// [AcquisitionServer] is currently only intended for use in local
	// development environments, so this implementation is ok for now.
	t.Run("concurrent-msg-image", func(t *testing.T) {
		t.Parallel()
		assert := assert.New(t)

		const nMessages = 24
		const nClients = 10

		// set up the servers
		mock := &mockAcq{}
		as, err := NewAcquisitionServer(mock)
		assert.Nil(err, err)
		assert.Nil(err, err)
		as.BufferSize = nMessages

		srv := httptest.NewServer(as)
		defer srv.Close()

		ctx, cancel := context.WithTimeout(context.Background(), time.Second*10)
		defer cancel()

		// get ready to acquire (and send) blobs
		msgs := make([]proto.Message, nMessages)
		// We're acquiring/generating and checking concurrently, so
		// we need to synchronize.
		var msgsMu sync.Mutex
		// We can't start generating blobs until one of the clients requests the
		// acquisition to start by sending a request to '/acquisition-start'.
		canAcquire := make(chan int, 1)
		canAcquireFn := sync.OnceFunc(func() {
			canAcquire <- 1
			close(canAcquire)
		})
		// We also have to signal to the clients not to stop acquisition,
		// via request to '/acquisition-stop', until all blobs have been
		// generated.
		acquisitionDone := make(chan int, 1)
		acquisitionDoneFn := sync.OnceFunc(func() { <-acquisitionDone })
		go func() {
			defer func() {
				acquisitionDone <- 1
				close(acquisitionDone)
			}()
			<-canAcquire
			for i := range msgs {
				msgsMu.Lock()
				msgs[i] = NewMessage(mock.acquire())
				msgsMu.Unlock()
			}
		}()

		// set up the clients
		clients := make([]*client, nClients)
		for i := range clients {
			clients[i], err = newClient(ctx, srv.URL, "/stream", srv.Client())
			assert.Nil(err, err)
			defer func() {
				err := clients[i].Close()
				assert.Nil(err, fmt.Sprintf("client[%v] close error: %v", i, err))
			}()
		}
		// Let the clients start/stop acquisition and check their messages
		// concurrently.
		var wg sync.WaitGroup
		for _, c := range clients {
			// start acquisition (should start only once, even when called
			// concurrently)
			wg.Add(1)
			go func() {
				defer wg.Done()
				defer canAcquireFn() // signal that we can start acquiring
				err := c.doHTTPRequest(ctx, http.MethodPost, "/acquisition-start", http.StatusOK)
				assert.Nil(err, err)
				// we can't actually enforce this without further synchronization
				// which seems unnecessary
				//err = mock.assertAcquisitionStarted()
				//assert.Nil(err, err)
			}()

			// read and check messages as they're comming in
			wg.Add(1)
			go func() {
				defer wg.Done()
				for i := range nMessages {
					msg, err := c.nextMessage()
					assert.Nil(err, err)

					// ok, because we can't receive more messages than we send
					msgsMu.Lock()
					assert.True(proto.Equal(msgs[i], msg))
					msgsMu.Unlock()
				}
			}()

			// stop acquisition only when all blobs have been generated
			wg.Add(1)
			go func() {
				defer wg.Done()
				acquisitionDoneFn()
				err := c.doHTTPRequest(ctx, http.MethodPost, "/acquisition-stop", http.StatusOK)
				assert.Nil(err, err)
				// we can't actually enforce this without further synchronization
				// which seems unnecessary
				//err = mock.assertAcquisitionStopped()
				//assert.Nil(err, err)
			}()

		}

		wg.Wait()
	})
}

func TestStartStopAcquisition(t *testing.T) {
	rootPath = "." // mount "/" at current working directory
	t.Parallel()
	assert := assert.New(t)

	const nClients = 10
	const nRequests = 100

	// set up the servers
	mock := &mockAcq{}
	as, err := NewAcquisitionServer(mock)
	assert.Nil(err, err)

	srv := httptest.NewServer(as)
	defer srv.Close()

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*10)
	defer cancel()

	// Set up evil clients and let them randomly start/stop acquisition.
	var wg sync.WaitGroup
	ch := make(chan bool, 1)
	for i := range nClients {
		c, err := newClient(ctx, srv.URL, "/stream", srv.Client())
		assert.Nil(err, err)
		defer func() {
			err := c.Close()
			assert.Nil(err, fmt.Sprintf("client[%v] close error: %v", i, err))
		}()
		wg.Add(nRequests)
		for _ = range nRequests {
			select {
			case ch <- true:
				<-ch
				go func() {
					defer wg.Done()
					err := c.doHTTPRequest(ctx, http.MethodPost, "/acquisition-start", http.StatusOK)
					assert.Nil(err, err)
				}()
			case ch <- false:
				<-ch
				go func() {
					defer wg.Done()
					err := c.doHTTPRequest(ctx, http.MethodPost, "/acquisition-stop", http.StatusOK)
					assert.Nil(err, err)
				}()
			}
		}
	}
	wg.Wait()
}
