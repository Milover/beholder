package server

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"time"

	pb "github.com/Milover/beholder/internal/server/proto"
	"github.com/coder/websocket"
	"google.golang.org/protobuf/proto"
)

// connection represents an active WebSocket connection
// to the [AcquisitionServer].
//
// A connection shouldn't be copied once constructed.
type connection struct {
	// addr is the originating address of the connection.
	addr string
	// msgs is the channel on which messages from the server are received.
	msgs chan []byte
	// closed reports whether the underlying connection has been closed.
	closed bool
	// mu is the underlying synchronization mechanism.
	mu sync.Mutex
	// c is the underlying WebSocket connection.
	c *websocket.Conn
}

// drop closes a connection with a status code and reports the reason.
func (c *connection) drop(code websocket.StatusCode, err error) error {
	c.mu.Lock()
	defer c.mu.Unlock()

	c.closed = true
	if c.c != nil {
		var reason string
		if err != nil {
			reason = err.Error()
		}
		return c.c.Close(code, reason)
	}
	return nil
}

// dropNow immediately closes a connection.
func (c *connection) dropNow() error {
	c.mu.Lock()
	defer c.mu.Unlock()

	c.closed = true
	if c.c != nil {
		return c.c.CloseNow()
	}
	return nil
}

// write marshals msg and writes it to c's underlying WebSocket connection.
func (c *connection) write(ctx context.Context, timeout time.Duration, msg []byte) error {
	writeCtx, cancelWrite := context.WithTimeout(ctx, timeout)
	defer cancelWrite()

	// this should never happen
	if msg == nil {
		return errors.New("nil message")
	}
	return c.c.Write(writeCtx, websocket.MessageBinary, msg)
}

// read reads and unmarshals a message from c's underlying WebSocket connection.
// If an error occurs a nil message is returned.
func (c *connection) read(ctx context.Context) (*pb.MessageWrapper, error) {
	typ, b, err := c.c.Read(ctx) // XXX: no timeout, not sure if ok
	if err != nil {
		return nil, err
	}
	if typ != websocket.MessageBinary {
		return nil, ErrMsg
	}
	msg := &pb.MessageWrapper{}
	if err := proto.Unmarshal(b, msg); err != nil {
		return nil, errors.Join(ErrMsg, err)
	}
	return msg, nil
}

// connError wraps an error and the connection on which it happened.
//
// It is intended to be used by consumers of connections, not by connections
// themselves.
type connError struct {
	err  error
	conn *connection
}

// newConnError wraps err into a connection error.
// If err is nil, the returned error is also nil.
func newConnError(err error, c *connection) error {
	if err == nil {
		return (*connError)(nil)
	}
	return &connError{err: err, conn: c}
}

// Error returns a string representation of a connError.
func (c *connError) Error() string {
	return fmt.Sprintf("%v (%p)", c.err, c.conn)
}

// Unwrap returns the error a connError wraps.
func (c *connError) Unwrap() error {
	return c.err
}
