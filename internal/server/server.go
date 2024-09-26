package server

import (
	"context"
	"errors"
	"log"
	"net"
	"net/http"
	"sync"
	"time"

	"github.com/coder/websocket"
	"github.com/coder/websocket/wsjson"
)

// AcquisitionStartStopper is the interface which wraps the
// StartAcquisition and AcquisitionStop methods.
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
	ID        string    `json:"id"`        // image ID
	Source    string    `json:"source"`    // source device ID, eg. camera S/N
	Timestamp time.Time `json:"timestamp"` // time at which the image was acquired
	Src       string    `json:"src"`       // image URL
}

// MessageType designates the type of payload of a message sent or received
// by the [AcquisitionServer].
type MessageType int

// The recognized message types.
const (
	MessageImage MessageType = iota + 1 // the
	MessageError
	MessageInfo
	MessageControl
)

// MessageVersion is the current message version.
// TODO: this should probably be handled in some other way.
const MessageVersion string = "v1.0.0"

// Message is the data type sent or received by the [AcquisitionServer].
type Message struct {
	Version string      `json:"version"`
	Type    MessageType `json:"type"`
	Payload any         `json:"payload"`
}

// connection represents an active WebSocket connection
// to the [AcquisitionServer].
//
// A connection shouldn't be copied once constructed.
type connection struct {
	// addr is the originating address of the connection.
	addr string
	// writeTimeout is the duration allowed for writing a message.
	writeTimeout time.Duration
	// msgs is the channel on which raw bytes of the message are sent.
	msgs chan *Message
	// closed reports whether the underlying connection has been closed.
	closed bool
	// mu is the synchronization mechanism
	mu sync.Mutex
	// c is the underlying WebSocket connection
	c *websocket.Conn
}

// drop closes a connection with a StatusPolicyViolation and reports the reason.
func (c *connection) drop(reason string) {
	c.mu.Lock()
	defer c.mu.Unlock()
	c.closed = true
	if c.c != nil {
		c.c.Close(websocket.StatusPolicyViolation, reason)
	}
}

// writeMsg encodes msg to JSON and writes it to c's underlying WebSocket
// connection.
//
// FIXME: this is no good because [Blob.Bytes] gets base64 encoded, which
// on average yields a 1/3 size increase.
func (c *connection) writeMsg(ctx context.Context, msg *Message) error {
	writeCtx, cancelWrite := context.WithTimeout(ctx, c.writeTimeout)
	defer cancelWrite()
	return wsjson.Write(writeCtx, c.c, msg)
}

// AcquisitionServer starts/stops image acquisition on (HTTP) request and
// streams acquired images via WebSocket to the front end.
//
// TODO: should turn this into a (middleware) muxer/Handler.
type AcquisitionServer struct {
	// BufferSize is the size of the [Blob] channel.
	//
	// Defaults to 8.
	BufferSize int
	// WriteTimeout is the maximum duration allowed for writing a message
	// to a connection.
	//
	// Defaults to 5 seconds.
	WriteTimeout time.Duration

	// acq handles starting and stopping image acquisition and sending
	// acquired images and/or errors on their respective channels.
	acq AcquisitionStartStopper
	// mux routes endpoints to their appropriate handlers.
	mux *http.ServeMux

	// blobs is the channel on which acquired images are received.
	blobs <-chan *Blob
	// errs is the channel on which acquisition errors are received.
	errs <-chan error

	// conns is a map of active connections.
	// FIXME: this is so dumb
	conns map[*connection]struct{}
	// connsMu is conns' synchronization mechanism.
	connsMu sync.Mutex
}

// NewAcquisitionServer creates a new AcquisitionServer ready to be used,
// if the acq is not nil.
func NewAcquisitionServer(acq AcquisitionStartStopper) (*AcquisitionServer, error) {
	if acq == nil {
		return nil, errors.New("server.NewAcquisitionServer: nil AcquisitionStartStopper")
	}
	s := &AcquisitionServer{
		BufferSize:   8,
		WriteTimeout: time.Second * 5,
		acq:          acq,
		mux:          http.NewServeMux(),
		conns:        make(map[*connection]struct{}),
	}
	s.mux.Handle("/", http.FileServer(http.Dir("web/app")))
	s.mux.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.Dir("web/static")))) // FIXME: should be configurable
	s.mux.HandleFunc("POST /acquisition-start", s.acquisitionStartHandler())
	s.mux.HandleFunc("POST /acquisition-stop", s.acquisitionStopHandler())
	s.mux.HandleFunc("/stream", s.streamHandler())
	// start receiving and distributing messages
	go func() {
		for {
			select {
			case blob := <-s.blobs:
				// If we're here, we either have a message to send, or
				// s.messages was closed (acquisition has stopped), in which
				// case we should check for errors.
				if blob == nil {
					// TODO: distribute encountered errors as messages
					if err, ok := <-s.errs; ok && err != nil {
						log.Printf("error: %v", err)
					}
					// if acquisition stopped regularly, than there's nothing to do
					continue
				}
				msg := &Message{
					Type:    MessageImage,
					Version: MessageVersion,
					Payload: blob,
				}
				s.distribute(msg)
			default:
				// This is necessary so that s.messages can get refreshed once
				// image acquisition starts, otherwise the select statement
				// retains a stale (possibly nil) s.messages and might block
				// indefinitely.
				time.Sleep(time.Millisecond * 10)
			}
		}
	}()
	return s, nil
}

// ServeHTTP dispatches the request to the handler whose pattern most closely
// matches the request URL.
func (s *AcquisitionServer) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	s.mux.ServeHTTP(w, r)
}

// acquireHandler handles the "/acquire-start" endpoint.
func (s *AcquisitionServer) acquisitionStartHandler() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		s.blobs, s.errs = s.acq.StartAcquisition(s.BufferSize)
		http.Redirect(w, r, "/", http.StatusSeeOther)
	}
}

// acquireHandler handles the "/acquire-stop" endpoint.
func (s *AcquisitionServer) acquisitionStopHandler() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		s.acq.StopAcquisition()
		http.Redirect(w, r, "/", http.StatusSeeOther)
	}
}

// streamHandler handles the WebSocket data stream.
func (s *AcquisitionServer) streamHandler() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		err := s.stream(w, r)
		if errors.Is(err, context.Canceled) {
			log.Println("websocket connection closed due to cancelled context")
			return
		}
		if websocket.CloseStatus(err) == websocket.StatusNormalClosure ||
			websocket.CloseStatus(err) == websocket.StatusGoingAway {
			log.Println("websocket connection closed")
			return
		}
		if err != nil {
			log.Printf("error: %v", err)
			return
		}
	}
}

// stream accepts a WebSocket connection and writes Blobs sent on
// [AcquisitionServer.messages] to it, consuming the sent [Blob].
// Errors sent on [AcquisitionServer.errs] are checked only once
// [AcquisitionServer.messages] is closed.
//
// TODO: stream uses [websocket.CloseRead] to keep reading from the connection
// to process control messages and cancel the context if the connection drops.
// However, [websocket.CloseRead] closes the connection once a data message is
// read. This is ok for now, but we would like to use the connection for
// back and forth communication, and as we understand it, [websocket.CloseRead]
// does not allow this.
func (s *AcquisitionServer) stream(w http.ResponseWriter, r *http.Request) error {
	conn := &connection{
		addr:         r.RemoteAddr,
		writeTimeout: s.WriteTimeout,
		msgs:         make(chan *Message, s.BufferSize),
	}
	s.addConnection(conn)
	defer s.deleteConnection(conn)

	// XXX: this entire block seems contrived, why can't we just assign
	// the socket to c?
	// It handles the case when closeSlow is called from somewhere, but
	// the setup hasn't even finished. So in effect we're dropping connections
	// which are deemed 'too slow' because of our connection setup being
	// too slow?
	// Maybe this handles cases when the handshake is taking too long?
	c, err := websocket.Accept(w, r, nil) // *AcceptOptions == nil
	if err != nil {
		return err
	}
	conn.mu.Lock()
	if conn.closed {
		conn.mu.Unlock()
		return net.ErrClosed
	}
	conn.c = c
	conn.mu.Unlock()
	defer conn.c.CloseNow()

	ctx := conn.c.CloseRead(context.Background()) // FIXME: ok for now, but doesn't work for us
	for {
		select {
		case msg := <-conn.msgs:
			if err := conn.writeMsg(ctx, msg); err != nil {
				return err
			}
		case <-ctx.Done():
			return ctx.Err()
		}
	}
}

// distribute sends msg to all active connections.
// It never blocks and so messages to slow connections are dropped.
func (s *AcquisitionServer) distribute(msg *Message) {
	s.connsMu.Lock()
	defer s.connsMu.Unlock()

	// should we rate limit here?
	for c := range s.conns {
		select {
		case c.msgs <- msg:
		default:
			go c.drop("connection too slow")
		}
	}
}

// addConnection registers a new active connection.
func (s *AcquisitionServer) addConnection(c *connection) {
	s.connsMu.Lock()
	s.conns[c] = struct{}{}
	s.connsMu.Unlock()
	log.Printf("registered new connection: %v", c.addr)
}

// deleteConnection deletes a connection.
func (s *AcquisitionServer) deleteConnection(c *connection) {
	s.connsMu.Lock()
	delete(s.conns, c)
	s.connsMu.Unlock()
	log.Printf("de-registered connection: %v", c.addr)
}
