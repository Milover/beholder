package output

import (
	"encoding/json"
	"io"

	"github.com/Milover/ocr/internal/ocr"
)

// Writer writes a formatted ocr.Result to an output target.
type Writer interface {
	Write(*ocr.Result) error
}

// WriteCloser wraps a Writer and an io.Closer.
type WriteCloser interface {
	Writer
	io.Closer
}

type Outputer struct {
	// Format is the format ocr.Result output format specifier.
	Format FormatType `json:"format"`
	// Target is the keyword specifier of the output target.
	Target TargetType `json:"target"`
	// Spec is the output target configuration.
	Spec json.RawMessage `json:"spec"`

	wc WriteCloser
}

// Init initializes the Outputer so it can be used, i.e.
// it initializes the output target and the ocr.Result writer.
func (o *Outputer) Init() error {
	wc, err := newTarget(o.Target, o.Spec)
	if err != nil {
		return err
	}
	o.wc, err = newFormat(o.Format, wc)
	return err
}

// Write writes a formatted ocr.Result to the output target.
func (o *Outputer) Write(r *ocr.Result) error {
	return o.wc.Write(r)
}

// Close closes the output target, at which point
// it can no longer be written to.
// Close() should always be called once Outputer is no longer required, so that
// all underlying resources are freed and the write buffer is flushed.
func (o *Outputer) Close() error {
	return o.wc.Close()
}
