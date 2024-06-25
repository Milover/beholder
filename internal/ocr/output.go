package ocr

import (
	"encoding/csv"
	"encoding/json"
	"errors"
	"io"
	"os"

	"github.com/Milover/ocr/internal/enumutils"
)

var (
	ErrBadOutputFormat       = errors.New("bad output format type")
	ErrBadOutputTarget       = errors.New("bad target type")
	ErrOutputTargetEmptyFile = errors.New("output target file name empty")
)

// ResultWriter writes a formatted Result to an output target.
type ResultWriter interface {
	Write(*Result) error
}

// Outputer wraps a ResultWriter and an io.Closer.
type Outputer interface {
	ResultWriter
	io.Closer
}

// Output outputs formatted Result(s) to an output target.
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
	}
}

// Init initializes the Output so it can be used, i.e.
// it initializes the output target and the Result writer.
func (o *Output) Init() error {
	wc, err := newOutTarget(o.Target, o.Spec)
	if err != nil {
		return err
	}
	o.o, err = newOutputer(o.Format, wc)
	return err
}

// Write writes a formatted Result to the output target.
func (o *Output) Write(r *Result) error {
	return o.o.Write(r)
}

// Close closes the output target, at which point
// it can no longer be written to.
// Close() should always be called once Output is no longer required, so that
// all underlying resources are freed and the write buffer is flushed.
func (o *Output) Close() error {
	return o.o.Close()
}

// OutFmtType represents available output formats.
type OutFmtType int

const (
	OFTNone OutFmtType = iota
	OFTCSV
	OFTJSON
)

var (
	outFmtTypeMap = map[OutFmtType]string{
		OFTNone: "none",
		OFTCSV:  "csv",
		OFTJSON: "json",
	}
	invOutFmtTypeMap = enumutils.Invert(outFmtTypeMap)
)

func (o *OutFmtType) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, o, invOutFmtTypeMap)
}

func (o OutFmtType) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(o, outFmtTypeMap)
}

// outputerFactory creates a new Outputer from an io.WriteCloser.
type outputerFactory func(io.WriteCloser) (Outputer, error)

// outputerFactoryMap is maps format types to Outputer factory functions.
var outputerFactoryMap = map[OutFmtType]outputerFactory{
	OFTNone: newOutNone,
	OFTCSV:  newOutCSV,
	OFTJSON: newOutJSON,
}

func newOutputer(o OutFmtType, wc io.WriteCloser) (Outputer, error) {
	switch o {
	case OFTNone:
		return newOutNone(wc)
	case OFTCSV:
		return newOutCSV(wc)
	case OFTJSON:
		return newOutJSON(wc)
	}
	return nil, ErrBadOutputFormat
}

// outNone is a dummy Outputer.
type outNone struct{}

func (o outNone) Write(*Result) error {
	return nil
}

func (o outNone) Close() error {
	return nil
}

// newOutNone creates a new dummy outputer.
func newOutNone(io.WriteCloser) (Outputer, error) {
	return outNone{}, nil
}

// outCSV writes CSV encoded output.
type outCSV struct {
	w *csv.Writer
	c io.Closer
	// hdr is a flag for keeping track if the CSV header was written.
	// The header gets written on the first write.
	hdr bool
}

func (o *outCSV) Write(r *Result) error {
	if !o.hdr {
		err := o.w.Write([]string{
			"timestamp",
			"expected",
			"result",
			"status",
		})
		if err != nil {
			return err
		}
		o.hdr = true
	}
	ts, err := r.TimeStamp.MarshalText()
	if err != nil {
		return err
	}
	return o.w.Write([]string{
		string(ts),
		r.Expected,
		r.Text,
		r.Status.String(),
	})
}

func (o outCSV) Close() error {
	o.w.Flush()
	if err := o.w.Error(); err != nil {
		return err
	}
	return o.c.Close()
}

// newOutCSV creates a new CSV Outputer.
func newOutCSV(wc io.WriteCloser) (Outputer, error) {
	return &outCSV{
		w: csv.NewWriter(wc),
		c: wc,
	}, nil
}

// outJSON writes JSON encoded output.
type outJSON struct {
	w *json.Encoder
	c io.Closer
}

func (o outJSON) Write(r *Result) error {
	return o.w.Encode(r)
}

func (o outJSON) Close() error {
	return o.c.Close()
}

// newOutJSON creates a new JSON Outputer.
func newOutJSON(wc io.WriteCloser) (Outputer, error) {
	return outJSON{
		w: json.NewEncoder(wc),
		c: wc,
	}, nil
}

// OutTargetType represents available output targets.
type OutTargetType int

const (
	OTTNone OutTargetType = iota
	OTTStdout
	OTTFile
)

var (
	outTargetTypeMap = map[OutTargetType]string{
		OTTNone:   "none",
		OTTStdout: "stdout",
		OTTFile:   "file",
	}
	intOutTargetTypeMap = enumutils.Invert(outTargetTypeMap)
)

func (o *OutTargetType) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, o, intOutTargetTypeMap)
}

func (o OutTargetType) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(o, outTargetTypeMap)
}

// outTargetFactory takes a JSON formatted output target spec and
// creates a new output target (io.WriteCloser) according to the spec.
type outTargetFactory func(json.RawMessage) (io.WriteCloser, error)

// ttFactoryMap is maps write targets to factory functions.
var outTargetFactoryMap = map[OutTargetType]outTargetFactory{
	OTTNone:   newOutTargetNone,
	OTTStdout: newOutTargetStdout,
	OTTFile:   newOutTargetFile,
}

func newOutTarget(t OutTargetType, m json.RawMessage) (io.WriteCloser, error) {
	switch t {
	case OTTNone:
		return newOutTargetNone(m)
	case OTTStdout:
		return newOutTargetStdout(m)
	case OTTFile:
		return newOutTargetFile(m)
	}
	return nil, ErrBadOutputTarget
}

// outTargetNone is a dummy output target.
type outTargetNone struct{}

func (o outTargetNone) Write(p []byte) (int, error) {
	return len(p), nil
}

func (o outTargetNone) Close() error {
	return nil
}

// newOutTargetNone creates a new dummy output target
// from a JSON formatted spec.
func newOutTargetNone(json.RawMessage) (io.WriteCloser, error) {
	return outTargetNone{}, nil
}

// outTargetStdout uses stdout as the output target.
type outTargetStdout struct {
	w *os.File
}

func (o outTargetStdout) Write(p []byte) (int, error) {
	return o.w.Write(p)
}

func (o outTargetStdout) Close() error {
	return nil
}

// newOutTargetStdout creates a new stdout output target
// from a JSON formatted spec.
func newOutTargetStdout(json.RawMessage) (io.WriteCloser, error) {
	return outTargetStdout{w: os.Stdout}, nil
}

// outTargetFile uses a file as the output target.
type outTargetFile struct {
	// File is the file path to which the output is written.
	File string `json:"file"`

	f *os.File
}

func (o outTargetFile) Write(p []byte) (int, error) {
	return o.f.Write(p)
}

func (o outTargetFile) Close() error {
	return o.f.Close()
}

// newOutTargetFile creates a new file output target
// from a JSON formatted spec.
func newOutTargetFile(m json.RawMessage) (io.WriteCloser, error) {
	var o outTargetFile
	var err error
	if err = json.Unmarshal(m, &o); err != nil {
		return o, err
	}
	if len(o.File) == 0 {
		return nil, ErrOutputTargetEmptyFile
	}
	// TODO: whould log warning or abort if the file exists
	if o.f, err = os.Create(o.File); err != nil {
		return o, err
	}
	return o, nil
}
