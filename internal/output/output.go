package output

import (
	"bufio"
	"encoding/csv"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"

	"github.com/Milover/beholder/internal/enumutils"
	"github.com/Milover/beholder/internal/ocr"
)

var (
	ErrBadOutputFormat       = errors.New("bad output format type")
	ErrBadOutputTarget       = errors.New("bad target type")
	ErrOutputTargetEmptyFile = errors.New("output target file name empty")
)

// ResultWriter is a buffered Result writer.
// It writes a formatted Results to an underlying output target (io.WriteCloser).
type ResultWriter interface {
	Write(*ocr.Result) error
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
func (o *Output) Write(r *ocr.Result) error {
	return o.o.Write(r)
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

func (o outNone) Write(*ocr.Result) error {
	return nil
}

func (o outNone) Close() error {
	return nil
}

func (o outNone) Flush() error {
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

func (o *outCSV) Write(r *ocr.Result) error {
	nLines := len(r.Text)
	record := make([]string, 0, 2+2*nLines)
	if !o.hdr {
		record = append(record, "timestamp")
		for i := range nLines {
			record = append(record, fmt.Sprintf("expected_l%d", i))
			record = append(record, fmt.Sprintf("result_l%d", i))
		}
		record = append(record, "status")
		if err := o.w.Write(record); err != nil {
			return err
		}
		record = record[:0]
		o.hdr = true
	}
	ts, err := r.TimeStamp.MarshalText()
	if err != nil {
		return err
	}
	record = append(record, string(ts))
	for i := range nLines {
		record = append(record, r.Expected[i])
		record = append(record, r.Text[i])
	}
	record = append(record, r.Status.String())
	return o.w.Write(record)
}

func (o outCSV) Close() error {
	err := o.Flush()
	return errors.Join(o.c.Close(), err)
}

func (o outCSV) Flush() error {
	o.w.Flush()
	return o.w.Error()
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
	b *bufio.Writer
}

func (o outJSON) Write(r *ocr.Result) error {
	return o.w.Encode(r)
}

func (o outJSON) Close() error {
	err := o.Flush()
	return errors.Join(o.c.Close(), err)
}

func (o outJSON) Flush() error {
	return o.b.Flush()
}

// newOutJSON creates a new JSON Outputer.
func newOutJSON(wc io.WriteCloser) (Outputer, error) {
	buf := bufio.NewWriter(wc)
	return outJSON{
		w: json.NewEncoder(buf),
		c: wc,
		b: buf,
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
