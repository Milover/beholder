package server

//go:generate protoc -I=proto/ --go_out=proto/ --go_opt=paths=source_relative proto/server.proto

import (
	"context"
	"errors"
	"log"
	"net"
	"net/http"
	"sync"
	"time"

	"github.com/Milover/beholder/internal/container/set"
	pb "github.com/Milover/beholder/internal/server/proto"
	"github.com/coder/websocket"
	"github.com/google/uuid"
	"google.golang.org/protobuf/proto"
)

var (
	ErrMsg = errors.New("bad message") // Invalid message
)

// Internal use errors.
var (
	errConnSlow = errors.New("connection too slow")

	errAcqStarted = errors.New("acquisition already started")
	errAcqStopped = errors.New("acquisition already stopped")
)

func init() {
	// Enable internal randomness pool to improve UUID generation throughput.
	//
	// FIXME: maybe think of a better place to put this, we're bound to move
	// some files around and forget that this is supposed to be turned on.
	uuid.EnableRandPool()
}

// AcquisitionStartStopper is the interface which wraps the
// StartAcquisition and AcquisitionStop methods.
//
// It is assumed that AcquisitionStartStopper implementations are not safe
// for concurrent use.
//
// WARNING: guys implementing this bad boy need thorough testing.
//
// TODO: using channels returned by [StartAcquisition] from within
// [StopAcquisition] is kind of scuffed, maybe we should rethink the interface
// structure.
type AcquisitionStartStopper interface {
	// IsAcquring reports whether an acquisition process is currently running.
	IsAcquiring() bool

	// StartAcquisition should start the image acquisition process, and return
	// a [Blob] channel blobs, on which acquired images will be sent, as well
	// as an error channel, on which (irrecoverable) acquisition errors
	// will be sent.
	// In this context, an irrecoverable acquisition error is implementation
	// defined, but refers errors which prevent further image acquisition,
	// eg. losing connection to the camera device.
	//
	// blobs should be of length blobsSize, while errs should be of length 1,
	// since once an error occurs, no further images should be acquired.
	//
	// While acquisition is running, blobs should be open, and once acquisition
	// stops, regularly or due to an error, blobs should be closed.
	// If an (irrecoverable) error occurs during acquisition, acquisition
	// should stop, blobs should be closed, and the error should be sent on errs.
	//
	// It is enough to close both blobs and errs to signal that acquisition
	// stopped regularly, eg. by a call to [StopAcquisition].
	//
	// Each call to StartAcquisition yields a new blobs and errs, which are
	// valid until the next call to StartAcquisition.
	//
	// Calling StartAcquisition when an acquisition process is running is not
	// an error, the 'currently active' blobs and errs should simply be
	// returned again.
	StartAcquisition(blobsSize int) (blobs <-chan *Blob, errs <-chan error)

	// StopAcquisition should stop the image acquisition process, and close
	// the [Blob] and error channels returned by [StartAcqsuition].
	//
	// Implementations should ensure that calling StopAcquisition stops
	// acquisition regularly, that is, without generating acquisition errors.
	// Calling StopAcquisition if acquisition is not running is not an error.
	//
	// NOTE: StopAcquisition is intended for 'manually' force-stopping
	// the acquisition process. However, [AcquisitionStartStopper]
	// implementations might stop acquisition automatically based on their
	// internal state, eg. after a certain number of images have been acquired,
	// hence, acquisition might get stopped regularly without StopAcquisition
	// having being called.
	StopAcquisition()
}

// Blob is the encoded acquired image.
//
// TODO: should probably turn this into a generic Message or something similar.
// TODO: the Blob could just embed or hold a models.Image
type Blob struct {
	UUID   uuid.UUID `json:"uuid"`   // image UUID v7 (embeds a timestamp)
	Source string    `json:"source"` // source device ID, eg. camera S/N
	Bytes  []byte    `json:"bytes"`  // encoded image bytes
}

// withPayload creates a new [proto.Message] with a payload.
// If the payload type is unsupported a message with a nil payload is returned.
//
// TODO: support payloads other than [*Blob] or generalize [Blob] to handle the
// different stuff we want to send.
func withPayload(payload any) *pb.MessageWrapper {
	switch p := payload.(type) {
	case *Blob:
		return &pb.MessageWrapper{
			Header: &pb.MessageHeader{
				Uuid: p.UUID.String(),
				Type: pb.MessageType_MESSAGE_TYPE_IMAGE,
			},
			Payload: &pb.MessageWrapper_Image{
				Image: &pb.Image{
					Source: p.Source,
					Mime:   http.DetectContentType(p.Bytes),
					Raw:    p.Bytes,
				},
			},
		}
	default:
		return &pb.MessageWrapper{
			Header: &pb.MessageHeader{
				Uuid: uuid.Must(uuid.NewV7()).String(),
				Type: pb.MessageType_MESSAGE_TYPE_UNKNOWN,
			},
		}
	}
}

// Route variables so they can be set during testing.
//
// TODO: could leave non-WebSocket/acquisition specific routes up to
// the top-level HTTP server, might make changing routes easier in the future.
var (
	// rootPath is the directory path at which content is hosted for "/".
	rootPath = "web/app"
	// imgsPath is the directory path at which content
	// is hosted for "/static/images/".
	imgsPath = "web/static/images"
)

// AcquisitionServer starts/stops image acquisition on (HTTP) request and
// streams acquired images via WebSocket to the front end.
//
// An AcquisitionServer shouldn't be copied once constructed.
//
// Call [AcquisitionServer.Close] when done to clean up and free resources.
//
// TODO: add Start/Close methods.
// TODO: should turn this into a (middleware) muxer/Handler.
// Could also break into two (or more) parts: one just for handling WebSocket
// communication and other ones which implement specific functionality,
// eg. acquisition control, then we could add new 'servers' as needed,
// each of which would implement some specific functionality.
type AcquisitionServer struct {
	// BlobsBufferSize is the size of the [Blob] channel used by the server and
	// each [connection].
	//
	// BlobsBufferSize is used (evaluated) only when acquisition is started,
	// hence if BlobsBufferSize is changed while acquisition is running, the
	// change will take effect the next time acquisition is started.
	//
	// Defaults to 8.
	BlobsBufferSize int
	// InboxBufferSize is the size of the [proto.Message] channel used by the
	// server.
	//
	// InboxBufferSize is used (evaluated) only when [AcquisitionServer.Start]
	// is called.
	//
	// Defaults to 32.
	InboxBufferSize int
	// WriteTimeout is the maximum duration allowed for writing a message
	// to a connection.
	//
	// WriteTimeout is used (evaluated) each time a connection writes a
	// message to it's underlying WebSocket connection.
	//
	// Defaults to 5 seconds.
	WriteTimeout time.Duration

	// Logf controls where logs are sent.
	//
	// Defaults to log.Printf.
	Logf func(f string, v ...interface{})

	// acq handles starting and stopping image acquisition and sending
	// acquired blobs (images) and/or errors on their respective channels.
	acq AcquisitionStartStopper
	// acqMu is acq's synchronization mechanism, since we don't want to impose
	// concurrency requirements on other people.
	acqMu sync.Mutex

	// inbox is the channel on which connections send messages.
	inbox chan request

	// conns are the currently active connections.
	conns set.Set[*connection]
	// connsMu is conns' synchronization mechanism.
	connsMu sync.Mutex

	// shutdown is a signal that the server is being shut down and
	// all resources should be freed.
	shutdown chan struct{}
	// shutdownMu is shutdown's synchronization mechanism.
	shutdownMu sync.Mutex

	// mux routes endpoints to their appropriate handlers.
	mux *http.ServeMux
}

// NewAcquisitionServer creates a new AcquisitionServer but doesn't start it.
//
// After changing its configuration, the caller should call
// [AcquisitionServer.Start].
//
// The caller should call [AcquisitionServer.Close] when done to clean up
// and free resources.
func NewAcquisitionServer(acq AcquisitionStartStopper) (*AcquisitionServer, error) {
	if acq == nil {
		return nil, errors.New("server.NewAcquisitionServer: nil AcquisitionStartStopper")
	}
	s := &AcquisitionServer{
		BlobsBufferSize: 8,
		InboxBufferSize: 32,
		WriteTimeout:    time.Second * 5,
		Logf:            log.Printf,
		acq:             acq,
		mux:             http.NewServeMux(),
		conns:           set.New[*connection](),
	}
	// FIXME: file server routes should probably be handled somewhere else
	s.mux.Handle("/", http.FileServer(http.Dir(rootPath)))
	s.mux.Handle("GET /static/images/{imgfile}", http.StripPrefix("/static/images/", http.FileServer(http.Dir(imgsPath)))) // TODO: can remove, no longer used
	s.mux.HandleFunc("GET /stream", s.streamHandler)
	// TODO: should be enabled only for testing/debugging
	s.mux.HandleFunc("POST /acquisition-start", s.acquisitionStartHandler)
	s.mux.HandleFunc("POST /acquisition-stop", s.acquisitionStopHandler)

	return s, nil
}

// Close closes all active connections, stops acquisition and
// frees all resources.
func (s *AcquisitionServer) Close() error {
	s.shutdownMu.Lock()
	defer s.shutdownMu.Unlock()

	s.Logf("server shutting down")
	if s.shutdown != nil {
		// the server has been started previously, signal the shutdown
		select {
		case <-s.shutdown:
			// already closed, nothing to do
		default:
			close(s.shutdown)
		}
	}
	// stop acquisition
	_ = s.stopAcquisition()

	// drop connections (stream() will clean up)
	s.connsMu.Lock()
	defer s.connsMu.Unlock()

	var err error
	for c := range s.conns {
		errors.Join(err, c.drop(websocket.StatusGoingAway, nil))
	}
	return err
}

// Start starts a server from [AcquisitionServer.NewAcquisitionServer]:
//   - incomming connections are accepted
//   - incomming messages from connections are received and processed
//
// Call [AcquisitionServer.Close] when done to clean up and free resources.
func (s *AcquisitionServer) Start() {
	s.shutdownMu.Lock()
	defer s.shutdownMu.Unlock()

	s.Logf("server starting")
	if s.shutdown != nil {
		// the server has been started previously
		select {
		case <-s.shutdown:
			// the server was closed, proceed with restart
		default:
			// the server was never closed, bail
			return
		}
	}
	s.inbox = make(chan request, s.InboxBufferSize)
	s.shutdown = make(chan struct{})

	// start receiving and processing messages from connections
	go func() {
		for {
			select {
			case <-s.shutdown:
				// server shutdown, bail
				return
			case req := <-s.inbox:
				resp, uuid, err := s.processRequest(req)
				// ignore the error if it's specifically a 'no-distribute' error
				if err != nil && err != ErrNoDistribute {
					s.Logf("%v (uuid: %v; conn: %p)", err, uuid, req.orig)
				}
				// if there is an error, only the request origin should receive
				// a response
				distribute := (err == nil)

				b, err := proto.Marshal(resp)
				if err != nil {
					s.Logf("failed to marshal response: %v (uuid: %v; conn: %p)",
						err, uuid, req.orig)
					continue
				}
				// queue the response
				if distribute {
					s.distribute(b)
				} else {
					s.sendTo(b, req.orig)
				}
				s.Logf("response queued (uuid: %v)", uuid)
			}
		}
	}()
}

// ServeHTTP dispatches the request to the handler whose pattern most closely
// matches the request URL.
func (s *AcquisitionServer) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	s.mux.ServeHTTP(w, r)
}

// acquireHandler handles the "/acquire-start" endpoint.
func (s *AcquisitionServer) acquisitionStartHandler(w http.ResponseWriter, r *http.Request) {
	_ = s.startAcquisition()
	http.Redirect(w, r, "/", http.StatusSeeOther)
}

// acquireHandler handles the "/acquire-stop" endpoint.
func (s *AcquisitionServer) acquisitionStopHandler(w http.ResponseWriter, r *http.Request) {
	_ = s.stopAcquisition()
	http.Redirect(w, r, "/", http.StatusSeeOther)
}

// streamHandler handles the WebSocket data stream.
func (s *AcquisitionServer) streamHandler(w http.ResponseWriter, r *http.Request) {
	err := s.stream(w, r)
	if errors.Is(err, context.Canceled) {
		s.Logf("websocket connection closed: %v", err)
		return
	}
	if websocket.CloseStatus(err) == websocket.StatusNormalClosure ||
		websocket.CloseStatus(err) == websocket.StatusGoingAway {
		s.Logf("websocket connection closed (%p)", err.(*connError).conn)
		return
	}
	if err != nil {
		s.Logf("server.AcquisitionServer.streamHandler: %v", err)
		return
	}
}

// startAcquisition safely sets up and starts acquisition.
// Incomming blobs are received and distributed to active connections.
//
// If there is an error, it will be an [errAcqStarted].
func (s *AcquisitionServer) startAcquisition() error {
	s.acqMu.Lock()
	defer s.acqMu.Unlock()

	if s.acq.IsAcquiring() {
		return errAcqStarted
	}
	blobs, errs := s.acq.StartAcquisition(s.BlobsBufferSize)
	// start receiving blobs and distributing them as messages
	go func() {
		for {
			select {
			case <-s.shutdown:
				// server shutdown, bail
				return
			case blob := <-blobs:
				if blob == nil {
					// The channel is closed and acquisition has stopped (see
					// [AcquisitionStartStopper]) so check for errors.
					// TODO: distribute encountered errors as messages
					if err, ok := <-errs; ok && err != nil {
						s.Logf("acquisition error: %v", err)
					}
					// acquisition stopped, bail
					return
				}
				msg := withPayload(blob)
				s.Logf("distributing message (uuid: %v; type: %v)",
					msg.Header.Uuid, msg.Header.Type)
				b, err := proto.Marshal(msg)
				if err != nil {
					s.Logf("could not marshal response message: %v", err)
					// marshalling failed, nothing to do
					continue
				}
				s.distribute(b)
			}
		}
	}()
	return nil
}

// stopAcquisition safely stops acquisition.
//
// If there is an error, it will be an [errAcqStopped].
func (s *AcquisitionServer) stopAcquisition() error {
	s.acqMu.Lock()
	defer s.acqMu.Unlock()

	// Calling IsAcquiring is not necessary, since it can never fail, but
	// we need to know the acquisition state before stopping it.
	if !s.acq.IsAcquiring() {
		return errAcqStopped
	}
	s.acq.StopAcquisition()

	return nil
}

// stream accepts a WebSocket connection and writes Blobs sent on
// [AcquisitionServer.messages] to it, consuming the sent [Blob].
// Errors sent on [AcquisitionServer.errs] are checked only once
// [AcquisitionServer.messages] is closed.
//
// If the returned error is not nil, it will be of type [*server.connError].
func (s *AcquisitionServer) stream(w http.ResponseWriter, r *http.Request) error {
	conn := &connection{
		addr: r.RemoteAddr,
		msgs: make(chan []byte, s.BlobsBufferSize),
	}
	s.register(conn)
	defer s.unregister(conn)

	// XXX: this entire block seems contrived, why can't we just assign
	// the socket to c?
	// It handles the case when [connection.drop] is called from somewhere, but
	// the setup hasn't even finished. So in effect we're dropping connections
	// which are deemed 'too slow' because of our connection setup being
	// too slow?
	// Maybe this handles cases when the handshake is taking too long?
	c, err := websocket.Accept(w, r, nil) // *AcceptOptions == nil
	if err != nil {
		return newConnError(err, conn)
	}
	conn.mu.Lock()
	if conn.closed {
		conn.mu.Unlock()
		return newConnError(net.ErrClosed, conn)
	}
	conn.c = c
	conn.mu.Unlock()
	defer conn.dropNow()

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// read messages from clients (and handle ping, pong and close frames)
	readErr := make(chan error, 1)
	go func() {
		for {
			msg, err := conn.read(ctx)
			if err != nil {
				if errors.Is(err, ErrMsg) {
					err = errors.Join(err, conn.drop(websocket.StatusUnsupportedData, ErrMsg))
				}
				readErr <- err
				// the connection will get (or already has been) dropped, bail
				return
			}
			s.Logf("read message (conn: %p)", conn.c) // XXX: shouldn't log here
			s.inbox <- request{orig: conn, msg: msg}
		}
	}()
	// write messages from the server
	for {
		select {
		case msg := <-conn.msgs:
			s.Logf("writing message (conn: %p)", conn.c) // XXX: shouldn't log here
			if err := conn.write(ctx, s.WriteTimeout, msg); err != nil {
				return newConnError(err, conn)
			}
		case err := <-readErr:
			return newConnError(err, conn)
		case <-ctx.Done():
			return newConnError(ctx.Err(), conn)
		}
	}
}

// distribute sends a message to all active connections.
// It never blocks, so slow connections are dropped.
func (s *AcquisitionServer) distribute(msg []byte) {
	s.connsMu.Lock()
	defer s.connsMu.Unlock()

	// should we rate-limit here?
	for c := range s.conns {
		select {
		case c.msgs <- msg:
		default:
			go c.drop(websocket.StatusPolicyViolation, errConnSlow) // FIXME: ignoring error
		}
	}
}

// sendTo sends a message to a single connection if it is active.
// It never blocks, so a slow connection will be dropped.
func (s *AcquisitionServer) sendTo(msg []byte, c *connection) {
	s.connsMu.Lock()
	defer s.connsMu.Unlock()

	_, found := s.conns[c]
	if found {
		select {
		case c.msgs <- msg:
		default:
			go c.drop(websocket.StatusPolicyViolation, errConnSlow) // FIXME: ignoring error
		}
	}
}

// register registers a new connection.
func (s *AcquisitionServer) register(c *connection) {
	s.connsMu.Lock()
	defer s.connsMu.Unlock()

	if c == nil {
		return
	}
	s.conns.Add(c)
	s.Logf("registered new connection: %v (%p)", c.addr, c)
}

// unregister unregisters a connection (but does NOT close it).
func (s *AcquisitionServer) unregister(c *connection) {
	s.connsMu.Lock()
	defer s.connsMu.Unlock()

	if c == nil {
		return
	}
	s.conns.Remove(c)
	s.Logf("unregistered connection: %v (%p)", c.addr, c)
}
