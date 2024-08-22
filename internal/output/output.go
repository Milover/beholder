package output

import (
	"encoding/json"
	"io"

	"github.com/Milover/beholder/internal/neutral"
)

// ResultWriter is a buffered Result writer.
// It writes a formatted Results to an underlying output target (io.WriteCloser).
type ResultWriter interface {
	Write(*neutral.Result) error
	Flush() error
}

// Outputer wraps a ResultWriter and an io.Closer.
type Outputer interface {
	ResultWriter
	io.Closer
}

// Output manages all resources needed for outputting Results to targets.
// It holds information needed to set up the Outputer and the output target,
// and manages the operation and life-cycle of these resources.
type Output struct {
	// Format is the format Result output format specifier.
	Format OutFmtType `json:"format"`
	// Target is the keyword specifier of the output target.
	Target OutTargetType `json:"target"`
	// Spec is the output target configuration.
	Spec json.RawMessage `json:"spec"`

	o Outputer
}

// NewOutput creates a new uninitialized Output with default values.
func NewOutput() *Output {
	return &Output{
		Target: OTTNone,
		Format: OFTNone,
		o:      outNone{},
	}
}

// Close closes the underlying output target and flushes the output buffer.
// After calling Close() all o can no longer write to the output target.
// Close() should always be called once Output is no longer required, so that
// all underlying resources are freed and the write buffer is flushed.
func (o *Output) Close() error {
	return o.o.Close()
}

// Flush flushes any buffered data to the underlying output target.
func (o *Output) Flush() error {
	return o.o.Flush()
}

// Init initializes the Output so it can be used, i.e.
// it initializes the underlying output target and Outputer.
func (o *Output) Init() error {
	wc, err := newOutTarget(o.Target, o.Spec)
	if err != nil {
		return err
	}
	o.o, err = newOutputer(o.Format, wc)
	return err
}

// Write writes a Result to the underlying output target using the
// underlying Outputer.
func (o *Output) Write(r *neutral.Result) error {
	return o.o.Write(r)
}
