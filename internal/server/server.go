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

// NewMessage creates a new Message from a payload.
// If the payload type is unsupported a message with a nil payload is returned.
//
// TODO: support payloads other than [*Blob] or generalize [Blob] to handle the
// different stuff we want to send.
func NewMessage(payload any) proto.Message {
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

// connection represents an active WebSocket connection
// to the [AcquisitionServer].
//
// A connection shouldn't be copied once constructed.
type connection struct {
	// addr is the originating address of the connection.
	addr string
	// msgs is the channel on which message are received.
	msgs chan proto.Message
	// closed reports whether the underlying connection has been closed.
	closed bool
	// mu is the underlying synchronization mechanism.
	mu sync.Mutex
	// c is the underlying WebSocket connection.
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

// write marshalls msg and writes it to c's underlying WebSocket connection.
func (c *connection) write(ctx context.Context, timeout time.Duration, msg proto.Message) error {
	writeCtx, cancelWrite := context.WithTimeout(ctx, timeout)
	defer cancelWrite()
	if msg == nil {
		return errors.New("nil protobuf message")
	}
	b, err := proto.Marshal(msg)
	if err != nil {
		return err
	}
	return c.c.Write(writeCtx, websocket.MessageBinary, b)
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
// TODO: should turn this into a (middleware) muxer/Handler.
// Could also break into two (or more) parts: one just for handling WebSocket
// communication and other ones which implement specific functionality,
// eg. acquisition control, then we could add new 'servers' as needed,
// each of which would implement some specific functionality.
type AcquisitionServer struct {
	// BufferSize is the size of the [Blob] channel used by the server and
	// each [connection].
	//
	// BufferSize is used (evaluated) only when acquisition is started,
	// hence if BufferSize is changed while acquisition is running, the
	// change will take effect the next time acquisition is started.
	//
	// Defaults to 8.
	BufferSize int
	// WriteTimeout is the maximum duration allowed for writing a message
	// to a connection.
	//
	// WriteTimeout is used (evaluated) each time a connection writes a
	// message to it's underlying WebSocket connection.
	//
	// Defaults to 5 seconds.
	WriteTimeout time.Duration

	// acq handles starting and stopping image acquisition and sending
	// acquired blobs (images) and/or errors on their respective channels.
	acq AcquisitionStartStopper
	// acqMu is acq's synchronization mechanism, since we don't want to impose
	// concurrency requirements on other people.
	acqMu sync.Mutex

	// blobs is the channel on which acquired blobs (images) are received.
	//
	// TODO: should turn into a channel of any, since stuff sent here
	// will get converted into a [proto.Message].
	blobs <-chan *Blob
	// errs is the channel on which acquisition errors are received.
	errs <-chan error

	// conns are the currently active connections.
	conns set.Set[*connection]
	// connsMu is conns' synchronization mechanism.
	connsMu sync.Mutex

	// mux routes endpoints to their appropriate handlers.
	mux *http.ServeMux
}

// NewAcquisitionServer creates a new AcquisitionServer ready to be used,
// if the acq is not nil.
//
// New messages can be received, and are distributed to any active connections,
// as soon as NewAcquisitionServer returns, provided no error occurs.
func NewAcquisitionServer(acq AcquisitionStartStopper) (*AcquisitionServer, error) {
	if acq == nil {
		return nil, errors.New("server.NewAcquisitionServer: nil AcquisitionStartStopper")
	}
	s := &AcquisitionServer{
		BufferSize:   8,
		WriteTimeout: time.Second * 5,
		acq:          acq,
		mux:          http.NewServeMux(),
		conns:        set.New[*connection](),
	}
	// FIXME: file server routes should probably be handled somewhere else
	s.mux.Handle("/", http.FileServer(http.Dir(rootPath)))
	s.mux.Handle("GET /static/images/{imgfile}", http.StripPrefix("/static/images/", http.FileServer(http.Dir(imgsPath))))
	s.mux.HandleFunc("POST /acquisition-start", s.acquisitionStartHandler)
	s.mux.HandleFunc("POST /acquisition-stop", s.acquisitionStopHandler)
	s.mux.HandleFunc("GET /stream", s.streamHandler)

	// start receiving and distributing messages
	go func() {
		var blobs <-chan *Blob
		var errs <-chan error
		for {
			s.acqMu.Lock()
			blobs = s.blobs
			errs = s.errs
			s.acqMu.Unlock()

			select {
			case blob := <-blobs:
				// If we're here, we either have a message to send, or
				// s.messages was closed (acquisition has stopped), in which
				// case we should check for errors.
				if blob == nil {
					// TODO: distribute encountered errors as messages
					if err, ok := <-errs; ok && err != nil {
						log.Printf("error: %v", err)
					}
					// bail, acquisition done
					continue
				}
				s.distribute(NewMessage(blob))
			default:
				// Necessary so that we don't block indefinitely.
				// NOTE: we could sleep here to leave some wiggle room for
				// other goroutines, however, even short sleep durations (1ms)
				// cause weird hangs in tests for some reason.
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
func (s *AcquisitionServer) acquisitionStartHandler(w http.ResponseWriter, r *http.Request) {
	s.startAcquisition()
	http.Redirect(w, r, "/", http.StatusSeeOther)
}

// acquireHandler handles the "/acquire-stop" endpoint.
func (s *AcquisitionServer) acquisitionStopHandler(w http.ResponseWriter, r *http.Request) {
	s.stopAcquisition()
	http.Redirect(w, r, "/", http.StatusSeeOther)
}

// streamHandler handles the WebSocket data stream.
func (s *AcquisitionServer) streamHandler(w http.ResponseWriter, r *http.Request) {
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

// startAcquisition safely sets up and starts acquisition.
func (s *AcquisitionServer) startAcquisition() {
	s.acqMu.Lock()
	defer s.acqMu.Unlock()

	if s.acq.IsAcquiring() {
		return
	}
	s.blobs, s.errs = s.acq.StartAcquisition(s.BufferSize)
}

// stopAcquisition safely stops acquisition.
func (s *AcquisitionServer) stopAcquisition() {
	s.acqMu.Lock()
	defer s.acqMu.Unlock()

	s.acq.StopAcquisition()
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
		addr: r.RemoteAddr,
		msgs: make(chan proto.Message, s.BufferSize),
	}
	s.addConnection(conn)
	defer s.deleteConnection(conn)

	// XXX: this entire block seems contrived, why can't we just assign
	// the socket to c?
	// It handles the case when [connection.drop] is called from somewhere, but
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

	// FIXME: ok for now, but doesn't work for two-way comms
	ctx := conn.c.CloseRead(context.Background())
	for {
		select {
		case msg := <-conn.msgs:
			if err := conn.write(ctx, s.WriteTimeout, msg); err != nil {
				return err
			}
		case <-ctx.Done():
			return ctx.Err()
		}
	}
}

// distribute sends msg to all active connections.
// It never blocks and so messages to slow connections are dropped.
func (s *AcquisitionServer) distribute(msg proto.Message) {
	s.connsMu.Lock()
	defer s.connsMu.Unlock()

	// should we rate-limit here?
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
	s.conns.Add(c)
	s.connsMu.Unlock()
	log.Printf("registered new connection: %v", c.addr)
}

// deleteConnection deletes a connection.
func (s *AcquisitionServer) deleteConnection(c *connection) {
	s.connsMu.Lock()
	s.conns.Remove(c)
	s.connsMu.Unlock()
	log.Printf("de-registered connection: %v", c.addr)
}
