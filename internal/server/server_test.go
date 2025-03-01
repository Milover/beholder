// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

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
	"github.com/stretchr/testify/require"
	assert "github.com/stretchr/testify/require"
	"golang.org/x/sync/errgroup"
	"google.golang.org/protobuf/proto"
)

// For future reference, the recommended command to run the tests is:
//
//	go test -v -race -count=N

// errMockAcq is the acquisition error used by mockAcq.
var errMockAcq = errors.New("acquisition error") //nolint:unused // see acquireErr

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
// If acq.acquiring == ture, it sends an [errMockAcq] on acq.errs and
// stops acquisition.
func (acq *mockAcq) acquireErr() { //nolint:unused // TODO: use or remove
	if acq.acquiring {
		acq.errs <- errMockAcq
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
// WARNING: assertAcquisitionStopped reads from both acq.blobs and acq.errs
// to check whether they are closed.
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
	defer resp.Body.Close() //nolint:errcheck // don't care

	_, _ = io.ReadAll(resp.Body)
	return resp.StatusCode
}

//
// HTTP handler tests
//

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

// TestRoutes tests the custom HTTP handlers of [AcquisitionServer].
// TODO: we should also test the file server handlers at some point, they
// seem like they could be a security risk.
// TODO: these should be autogenerated at some point.
func TestRoutes(t *testing.T) {
	t.Parallel()
	for _, tt := range routeTests {
		t.Run(strings.TrimPrefix(tt.route, "/"), func(t *testing.T) {
			t.Parallel()
			assert := assert.New(t)
			require := require.New(t)

			// set up the servers
			mock := &mockAcq{}
			as, err := NewAcquisitionServer(mock)
			require.NoError(err)
			as.Start()
			defer func() {
				err := as.Close()
				require.NoError(err)
			}()

			// run setup
			if tt.setup != nil {
				err := tt.setup(as, mock)
				require.NoError(err)
			}

			// test the route
			statusCode := sendRequest(tt.method, tt.route,
				func(w http.ResponseWriter, r *http.Request) {
					tt.handler(as, w, r)
				})
			assert.Equal(tt.expStatus, statusCode)

			err = tt.pass(as, mock)
			require.NoError(err)
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
//
//nolint:unparam // not changing signature
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
func (cl *client) nextMessage(ctx context.Context) (proto.Message, error) {
	typ, b, err := cl.c.Read(ctx)
	if err != nil {
		return nil, err
	}
	if typ != websocket.MessageBinary {
		cl.c.Close(websocket.StatusUnsupportedData, "expected binary message") //nolint:errcheck // don't care
		return nil, fmt.Errorf("expected binary message, but got: %v", typ)
	}
	// unmarshall the message
	msg := &pb.MessageWrapper{}
	if err := proto.Unmarshal(b, msg); err != nil {
		return nil, err
	}
	return msg, nil
}

//nolint:unparam // not changing signature
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
	defer resp.Body.Close() //nolint:errcheck // don't care

	_, err = io.ReadAll(resp.Body)
	if err != nil {
		return err
	}
	if resp.StatusCode != status {
		return fmt.Errorf("expected status %v, but got: %v", status, resp.StatusCode)
	}
	return nil
}

// TestStartStopAcquisition verifies that no data races occur when multiple
// clients start/stop acquisition concurrently.
// NOTE: assumes the test is run with the '-race' flag set.
func TestStartStopAcquisition(t *testing.T) {
	rootPath = "." // mount "/" at current working directory
	t.Parallel()
	require := require.New(t)

	const nClients = 10
	const nRequests = 100

	// set up the servers
	mock := &mockAcq{}
	as, err := NewAcquisitionServer(mock)
	require.NoError(err)
	as.Start()
	defer func() {
		err := as.Close()
		require.NoError(err)
	}()
	srv := httptest.NewServer(as)
	defer srv.Close()

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*10)
	defer cancel()

	// Set up evil clients and let them randomly start/stop acquisition.
	g := new(errgroup.Group)
	ch := make(chan bool, 1)
	for i := range nClients {
		c, err := newClient(ctx, srv.URL, "/stream", srv.Client())
		require.NoError(err)
		defer func() {
			err := c.Close()
			require.NoErrorf(err, "client[%v] close error: %v", i, err)
		}()
		for range nRequests {
			select {
			case ch <- true:
				<-ch
				g.Go(func() error {
					return c.doHTTPRequest(ctx, http.MethodPost, "/acquisition-start", http.StatusOK)
				})
			case ch <- false:
				<-ch
				g.Go(func() error {
					return c.doHTTPRequest(ctx, http.MethodPost, "/acquisition-stop", http.StatusOK)
				})
			}
		}
	}
	err = g.Wait()
	require.NoError(err)
}

//
// Image-messge handling tests
//

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
	t.Run("simple-send-msg-image", func(t *testing.T) {
		t.Parallel()
		assert := assert.New(t)
		require := require.New(t)

		// set up the servers
		mock := &mockAcq{}
		as, err := NewAcquisitionServer(mock)
		require.NoError(err)
		as.Start()
		defer func() {
			err := as.Close()
			require.NoError(err)
		}()
		srv := httptest.NewServer(as)
		defer srv.Close()

		// set up the client
		ctx, cancel := context.WithTimeout(context.Background(), time.Second*10)
		defer cancel()

		client, err := newClient(ctx, srv.URL, "/stream", srv.Client())
		require.NoError(err)
		defer func() {
			err := client.Close()
			require.NoErrorf(err, "client close error: %v", err)
		}()

		// start acquisition
		err = client.doHTTPRequest(ctx, http.MethodPost, "/acquisition-start", http.StatusOK)
		require.NoError(err)
		err = mock.assertAcquisitionStarted()
		require.NoError(err)

		// acquire (and send) blobs
		expMsg := withPayload(mock.acquire())

		// read and check the message
		msg, err := client.nextMessage(ctx)
		require.NoError(err)
		assert.True(proto.Equal(expMsg, msg))

		// stop acquisition
		err = client.doHTTPRequest(ctx, http.MethodPost, "/acquisition-stop", http.StatusOK)
		require.NoError(err)
		err = mock.assertAcquisitionStopped()
		require.NoError(err)
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
	t.Run("concurrent-send-msg-image", func(t *testing.T) {
		t.Parallel()
		require := require.New(t)

		const nMessages = 24
		const nClients = 10

		// set up the servers
		mock := &mockAcq{}
		as, err := NewAcquisitionServer(mock)
		require.NoError(err)
		as.ConnBufferSize = max(as.ConnBufferSize, nMessages) // to avoid 'connection too slow' failures
		as.Start()
		defer func() {
			err := as.Close()
			require.NoError(err)
		}()
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
		canAcquire := make(chan struct{})
		canAcquireFn := sync.OnceFunc(func() { close(canAcquire) })
		// We also have to signal to the clients not to stop acquisition,
		// via request to '/acquisition-stop', until all blobs have been
		// generated.
		acquisitionDone := make(chan struct{})
		go func() {
			defer close(acquisitionDone)
			<-canAcquire
			for i := range msgs {
				msgsMu.Lock()
				msgs[i] = withPayload(mock.acquire())
				msgsMu.Unlock()
			}
		}()

		// set up the clients
		clients := make([]*client, nClients)
		for i := range clients {
			clients[i], err = newClient(ctx, srv.URL, "/stream", srv.Client())
			require.NoError(err)
			defer func() {
				err := clients[i].Close()
				require.NoErrorf(err, "client[%v] close error: %v", i, err)
			}()
		}
		// Let the clients start/stop acquisition and check their messages
		// concurrently.
		g := new(errgroup.Group)
		for _, c := range clients {
			// start acquisition (should start only once, even when called
			// concurrently)
			g.Go(func() error {
				defer canAcquireFn() // signal that we can start generating blobs
				return c.doHTTPRequest(ctx, http.MethodPost, "/acquisition-start", http.StatusOK)
				// mock.assertAcquisitionStarted() needs further synchronization
			})
			// read and check messages as they're coming in
			g.Go(func() error {
				var err error
				for i := range nMessages {
					msg, errClient := c.nextMessage(ctx)
					err = errors.Join(err, errClient)

					// ok, because we can't receive more messages than we send
					msgsMu.Lock()
					if !proto.Equal(msgs[i], msg) {
						err = errors.Join(err, fmt.Errorf(
							"bad msg:\nexpected:\n%v\nbut got:\n%v", msgs[i], msg))
					}
					msgsMu.Unlock()
				}
				return err
			})
			// stop acquisition (should stop only once, even when called
			// concurrently)
			g.Go(func() error {
				<-acquisitionDone // wait until all blobs have been generated
				return c.doHTTPRequest(ctx, http.MethodPost, "/acquisition-stop", http.StatusOK)
				// mock.assertAcquisitionStopped() needs further synchronization
			})
		}
		err = g.Wait()
		require.NoError(err)
	})
}

//
// Op-messge request handling tests
//
// TODO: move these to a separate file.
//

// opMessageTest is a structure used for testing [AcquisitionServer] op-message
// request handling.
type opMsgTest struct {
	// Name is the test name
	Name string
	// OpCode is the opcode of the op-message request used in the test.
	OpCode pb.OpCode
	// Payload generates the expected payload of the response.
	Payload func() proto.Message
	// ObsPayload generates the expected payload of the 'observer' response.
	ObsPayload func() proto.Message
	// PreCheck sets up/checks additional stuff after the normal test setup.
	PreCheck func(*testing.T, *AcquisitionServer, *mockAcq)
	// PostCheck sets up/checks additional stuff before the test ends.
	PostCheck func(*testing.T, *AcquisitionServer, *mockAcq)
}

// request generates a new op-message request from tst.OpCode.
func (tst *opMsgTest) request() *pb.MessageWrapper {
	return &pb.MessageWrapper{
		Header: &pb.MessageHeader{
			Uuid: uuid.Must(uuid.NewV7()).String(),
			Type: pb.MessageType_MESSAGE_TYPE_OP,
		},
		Payload: &pb.MessageWrapper_Op{
			Op: &pb.Op{
				Header: &pb.OpHeader{
					Type: pb.OpMessageType_OP_MESSAGE_TYPE_REQUEST,
					Code: tst.OpCode,
				},
			},
		},
	}
}

// response generates a response from a request.
// If tst.Payload is defined, then the response is an op-messge response
// with a payload set to the result of tst.Payload.
// If tst.Resp is defined, then the response is the result of tst.Resp.
// If both, or neither, tst.Payload and tst.Resp are set, response panics.
func (tst *opMsgTest) response(req *pb.MessageWrapper, payload func() proto.Message) *pb.MessageWrapper {
	if payload == nil {
		panic("nil payload")
	}
	resp := (proto.Clone(req)).(*pb.MessageWrapper)
	pl := (resp.Payload).(*pb.MessageWrapper_Op)

	pl.Op.Header.Type = pb.OpMessageType_OP_MESSAGE_TYPE_RESPONSE
	switch p := (tst.Payload()).(type) {
	case *pb.Error:
		pl.Op.Payload = &pb.Op_Error{Error: p}
	default:
		panic("bad op payload")
	}
	return resp
}

var opMsgTests = []opMsgTest{
	{
		Name:   "msg-start-acquisition",
		OpCode: pb.OpCode_OP_CODE_START_ACQUISITION,
		Payload: func() proto.Message {
			return &pb.Error{
				Code:        pb.ErrorCode_ERROR_CODE_SUCCESS,
				Description: "acquisition started",
			}
		},
		ObsPayload: func() proto.Message {
			return &pb.Error{
				Code:        pb.ErrorCode_ERROR_CODE_FAIL,
				Description: ErrPermissions.Error(),
			}
		},
		PostCheck: func(t *testing.T, _ *AcquisitionServer, mock *mockAcq) {
			err := mock.assertAcquisitionStarted()
			require.NoError(t, err)
		},
	},
	{
		Name:   "msg-stop-acquisition",
		OpCode: pb.OpCode_OP_CODE_STOP_ACQUISITION,
		Payload: func() proto.Message {
			return &pb.Error{
				Code:        pb.ErrorCode_ERROR_CODE_SUCCESS,
				Description: "acquisition stopped",
			}
		},
		ObsPayload: func() proto.Message {
			return &pb.Error{
				Code:        pb.ErrorCode_ERROR_CODE_FAIL,
				Description: ErrPermissions.Error(),
			}
		},
		PreCheck: func(t *testing.T, _ *AcquisitionServer, mock *mockAcq) {
			_, _ = mock.StartAcquisition(0)
			err := mock.assertAcquisitionStarted()
			require.NoError(t, err)
		},
		PostCheck: func(t *testing.T, _ *AcquisitionServer, mock *mockAcq) {
			err := mock.assertAcquisitionStopped()
			require.NoError(t, err)
		},
	},
}

// TestMessageOpUnknown verifies that the server can receive and respond to
// op-messge requests from multiple active WebSocket connections concurrently
// without data races or errors.
//
// The test runs twice: once with a single client sending a single message,
// and a second time with multiple clients sending lots of messages
// concurrently.
//
// The client(s) always send an op-message request of unknown type, to which
// the server should respond with an op-message error response.
func TestMessageOpBasic(t *testing.T) {
	t.Parallel()
	testVers := []struct {
		clis int // number of clients used in test
		msgs int // number of messages each client sends/receives
	}{
		{1, 1},
		{16, 128},
	}
	tst := opMsgTest{
		Name:   "msg-unknown",
		OpCode: pb.OpCode_OP_CODE_UNKNOWN,
		Payload: func() proto.Message {
			return &pb.Error{
				Code:        pb.ErrorCode_ERROR_CODE_FAIL,
				Description: ErrMsg.Error(),
			}
		},
	}

	for _, tv := range testVers {
		name := fmt.Sprintf("%v-cli-%d-msg-%d", tst.Name, tv.clis, tv.msgs)
		t.Run(name, func(t *testing.T) {
			t.Parallel()
			require := require.New(t)

			// setup
			mock := &mockAcq{}
			as, err := NewAcquisitionServer(mock)
			require.NoError(err)
			as.ConnBufferSize = max(as.ConnBufferSize, tv.msgs) // to avoid 'connection too slow' failures
			as.Start()
			defer func() {
				err := as.Close()
				require.NoError(err)
			}()
			srv := httptest.NewServer(as)
			defer srv.Close()

			// set up the clients
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*10)
			defer cancel()

			clients := make([]*client, tv.clis)
			for i := range tv.clis {
				clients[i], err = newClient(ctx, srv.URL, "/stream", srv.Client())
				require.NoError(err)
				defer func() {
					err := clients[i].Close()
					require.NoError(err)
				}()
			}

			// construct request/response
			req := tst.request()
			expResp := tst.response(req, tst.Payload)
			reqBuf, err := proto.Marshal(req)
			require.NoError(err)

			// send and receive/check messages
			g := new(errgroup.Group)
			for _, client := range clients {
				// send messages
				g.Go(func() error {
					var err error
					for range tv.msgs {
						err = errors.Join(err,
							client.c.Write(ctx, websocket.MessageBinary, reqBuf))
					}
					return err
				})
				// receive and check messages
				g.Go(func() error {
					var err error
					for range tv.msgs {
						// read and check the server response message
						resp, errClient := client.nextMessage(ctx)
						err = errors.Join(err, errClient)
						if !proto.Equal(expResp, resp) {
							err = errors.Join(err, fmt.Errorf(
								"bad response:\nexpected:\n%v\nbut got:\n%v", expResp, resp))
						}
					}
					return err
				})
			}
			err = g.Wait()
			require.NoError(err)
		})
	}
}

// TestMessageOpPermissions verifies that the server receives and responds
// to service request messages sent by 'controller' and 'observer' clients
// through WebSocket connections correctly.
//
// The response from a 'controller' request must be distributed to all
// 'observers', while 'observer' responses should be sent only to the
// 'observer' that sent the request.
//
// Both the 'controller' and the 'observers' send the same request, however
// the 'observer' requests are responded to with a permission error, while
// 'controller' responses should be successful and distributed to all clients.
//
// The tests are as follows:
//  1. 'msg-start-acquisition verifies that the server responds to a
//     start-acquisition op-message request with a 'success' op-message and
//     starts acquisition.
//  2. 'msg-stop-acquisition' verifies that the server responds to a
//     stop-acquisition op-message request with a 'success' op-message and
//     stops acquisition.
//
// TODO: these should be autogenerated at some point.
func TestMessageOpPermissions(t *testing.T) {
	t.Parallel()

	clis := 10 // number of clients, the first client is the 'controller'
	for _, tt := range opMsgTests {
		t.Run(tt.Name, func(t *testing.T) {
			t.Parallel()
			require := require.New(t)

			// setup
			mock := &mockAcq{}
			as, err := NewAcquisitionServer(mock)
			require.NoError(err)
			as.Start()
			defer func() {
				err := as.Close()
				require.NoError(err)
			}()
			srv := httptest.NewServer(as)
			defer srv.Close()

			// set up the clients
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*10)
			defer cancel()

			clients := make([]*client, clis)
			for i := range clis {
				clients[i], err = newClient(ctx, srv.URL, "/stream", srv.Client())
				require.NoError(err)
				defer func() {
					err := clients[i].Close()
					require.NoError(err)
				}()
			}

			// run pre-check
			if tt.PreCheck != nil {
				tt.PreCheck(t, as, mock)
			}

			// construct request/response
			req := tt.request()
			ctrlResp := tt.response(req, tt.Payload)
			obsResp := tt.response(req, tt.ObsPayload)

			reqBuf, err := proto.Marshal(req)
			require.NoError(err)

			// send and receive/check messages
			g := new(errgroup.Group)
			for i, client := range clients {
				// send messages
				g.Go(func() error {
					return client.c.Write(ctx, websocket.MessageBinary, reqBuf)
				})
				// receive and check messages
				g.Go(func() error {
					if i == 0 {
						// the first client is the 'controller' and receives
						// a response only to it's own request
						resp, err := client.nextMessage(ctx)
						if !proto.Equal(ctrlResp, resp) {
							err = errors.Join(err, fmt.Errorf(
								"bad controler response:\nexpected:\n%v\nbut got:\n%v",
								ctrlResp, resp))
						}
						return err
					} else { // everyone else is an 'observer'
						// everyone else is an 'observer' and receives both
						// their own response, and the 'controller' response,
						// but we don't know in which order
						gotCtrlResp, gotObsResp := false, false
						var err error
						for range 2 {
							resp, errClient := client.nextMessage(ctx)
							err = errors.Join(err, errClient)
							gotCtrlResp = gotCtrlResp || proto.Equal(ctrlResp, resp)
							gotObsResp = gotObsResp || proto.Equal(obsResp, resp)
						}
						if !(gotCtrlResp && gotObsResp) {
							err = errors.Join(err, errors.New("bad observer response"))
						}
						return err
					}
				})
			}
			err = g.Wait()
			require.NoError(err)

			// run post-check
			if tt.PostCheck != nil {
				tt.PostCheck(t, as, mock)
			}
		})
	}
}
