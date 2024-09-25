package server

import (
	"context"
	"errors"
	"io"
	"log"
	"net/http"
	"time"

	"github.com/coder/websocket"
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
	StartAcquisition(blobsSize int) (blobs <-chan Blob, errs <-chan error)

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
type Blob struct {
	ID        string    // image ID	// XXX: or int/uint64?
	SourceID  string    // source device ID, eg. camera S/N
	Timestamp time.Time // the time at which the image was acquired
	MIME      string    // image MIME type
	Bytes     []byte    // raw image bytes
}

// AcquisitionServer starts/stops image acquisition on (HTTP) request and
// streams acquired images via WebSocket to the front end.
//
// TODO: should turn this into a (middleware) muxer/Handler.
type AcquisitionServer struct {
	// BufferSize is the size of the [Blob] channel.
	BufferSize int

	acq   AcquisitionStartStopper
	blobs <-chan Blob
	errs  <-chan error
	// mux routes endpoints to their appropriate handlers.
	mux *http.ServeMux
}

func NewAcquisitionServer(acq AcquisitionStartStopper, bufferSize int) (*AcquisitionServer, error) {
	if acq == nil {
		return nil, errors.New("server.NewAcquisitionServer: nil AcquisitionStartStopper")
	}
	s := &AcquisitionServer{
		BufferSize: bufferSize,
		acq:        acq,
		mux:        http.NewServeMux(),
	}
	s.mux.Handle("/", http.FileServer(http.Dir("web/app")))
	s.mux.HandleFunc("POST /acquisition-start", s.acquisitionStartHandler())
	s.mux.HandleFunc("POST /acquisition-stop", s.acquisitionStopHandler())
	s.mux.HandleFunc("/stream", s.streamHandler())
	//s.mux.HandleFunc("GET /stream", streamHandler(app))
	return s, nil
}

func (s *AcquisitionServer) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	s.mux.ServeHTTP(w, r)
}

// acquireHandler handles the "/acquire-start" endpoint.
func (s *AcquisitionServer) acquisitionStartHandler() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		log.Println("handling '/acquire-start'")

		s.blobs, s.errs = s.acq.StartAcquisition(s.BufferSize)
		http.Redirect(w, r, "/", http.StatusSeeOther)
	}
}

// acquireHandler handles the "/acquire-stop" endpoint.
func (s *AcquisitionServer) acquisitionStopHandler() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		log.Println("handling '/acquire-stop'")

		s.acq.StopAcquisition()
		http.Redirect(w, r, "/", http.StatusSeeOther)
	}
}

// streamHandler handles the WebSocket data stream.
func (s *AcquisitionServer) streamHandler() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		log.Println("handling '/stream'")

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
// [AcquisitionServer.blobs] to it, consuming the sent [Blob].
// Errors sent on [AcquisitionServer.errs] are checked only once
// [AcquisitionServer.blobs] is closed.
//
// FIXME: we write (and consume) a [Blob] sent on [AcquisitionServer.blobs]
// to a single connection only (the one stream sets up), when instead we should
// write the [Blob] to all active connections.
//
// TODO: [AcquisitionServer.blobs] and [AcquisitionServer.errs] get destroyed
// and reallocated when acquisition is started,
// see [AcquisitionStartStopper.StartAcquisition], so not sure how this
// interacts with the select statement, i.e. we might need a mutex to prevent
// weird behaviour.
// TODO: stream uses [websocket.CloseRead] to keep reading from the connection
// to process control messages and cancel the context if the connection drops.
// However, [websocket.CloseRead] closes the connection once a data message is
// read. This is ok for now, but we would like to use the connection for
// back and forth communication, and as we understand it, [websocket.CloseRead]
// does not allow this.
func (s *AcquisitionServer) stream(w http.ResponseWriter, r *http.Request) error {
	// defer: check errs chan
	c, err := websocket.Accept(w, r, nil) // *AcceptOptions == nil
	if err != nil {
		return err
	}
	defer c.CloseNow() // XXX: not sure if we should close here

	ctx := c.CloseRead(context.Background()) // FIXME: ok for now, but doesn't work for us
	for {
		select {
		case blob := <-s.blobs:
			log.Println("got a blob")
			// If we're here, we either have a blob to send, or s.blobs was
			// closed (acquisition has stopped), in which case we should
			// check for errors.
			if blob.Bytes == nil { // XXX: this is kinda ugly
				if err, ok := <-s.errs; ok && err != nil {
					return err
				}
				// if acquisition stopped regularly, than there's nothing to do
				continue
			}
			// TODO: the timeout should be configurable
			if err := writeBlob(ctx, time.Second*5, c, blob); err != nil {
				return err
			}
		case <-ctx.Done():
			return ctx.Err()
		default:
			time.Sleep(time.Millisecond * 50)
		}
	}
}

// writeBlob writes a [Blob] to c.
//
// TODO: only [Blob.Bytes], i.e. bytes of the acquired image, is written to c.
// Instead, the entire [Blob], which might hold additional data (eg. MIME type,
// camera ID...), should be encoded/marshalled and sent to the front end.
func writeBlob(ctx context.Context, timeout time.Duration, c *websocket.Conn, blob Blob) (err error) {
	log.Println("writing blob")

	writeCtx, cancelWrite := context.WithTimeout(ctx, timeout)
	defer cancelWrite()

	var wc io.WriteCloser
	wc, err = c.Writer(writeCtx, websocket.MessageBinary)
	if err != nil {
		return err
	}
	defer func() { // XXX: this is kinda ugly and cryptic
		err = errors.Join(err, wc.Close())
	}()

	_, err = wc.Write(blob.Bytes)
	return err
}
