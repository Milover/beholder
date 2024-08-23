package output

import (
	"bufio"
	"encoding/csv"
	"encoding/json"
	"errors"
	"fmt"
	"io"

	"github.com/Milover/beholder/internal/enumutils"
	"github.com/Milover/beholder/internal/neutral"
)

var (
	ErrBadOutputFormat = errors.New("bad output format type")
)

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

func (o outNone) Write(*neutral.Result) error {
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

func (o *outCSV) Write(r *neutral.Result) error {
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

func (o outJSON) Write(r *neutral.Result) error {
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
