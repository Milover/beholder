package output

import (
	"encoding/csv"
	"encoding/json"
	"errors"
	"io"

	"github.com/Milover/ocr/internal/enumutils"
	"github.com/Milover/ocr/internal/ocr"
)

var (
	ErrBadFormat = errors.New("bad format type")
)

// FormatType represents available output formats.
type FormatType int

const (
	FTNone FormatType = iota
	FTCSV
	FTJSON
)

var (
	formatTypeMap = map[FormatType]string{
		FTNone: "none",
		FTCSV:  "csv",
		FTJSON: "json",
	}
	invFormatTypeMap = enumutils.Invert(formatTypeMap)
)

func (f *FormatType) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, f, invFormatTypeMap)
}

func (f FormatType) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(f, formatTypeMap)
}

// wcFactory creates a new WriteCloser from an io.WriteCloser.
type wcFactory func(io.WriteCloser) (WriteCloser, error)

// wcFactoryMap is maps format types to WriteCloser factory functions.
var wcFactoryMap = map[FormatType]wcFactory{
	FTNone: newFTNone,
	FTCSV:  newFTCSV,
	FTJSON: newFTJSON,
}

func newFormat(t FormatType, wc io.WriteCloser) (WriteCloser, error) {
	switch t {
	case FTNone:
		return newFTNone(wc)
	case FTCSV:
		return newFTCSV(wc)
	case FTJSON:
		return newFTJSON(wc)
	}
	return nil, ErrBadFormat
}

// ftNone is a dummy encoder.
type ftNone struct{}

func (t ftNone) Write(*ocr.Result) error {
	return nil
}

func (t ftNone) Close() error {
	return nil
}

// newFTNone creates a new ftNone.
func newFTNone(io.WriteCloser) (WriteCloser, error) {
	return ftNone{}, nil
}

// ftCSV writes CSV encoded output.
type ftCSV struct {
	w *csv.Writer
	c io.Closer
	// hdr is a flag for keeping track if the CSV header was written.
	// The header gets written on the first write.
	hdr bool
}

func (t *ftCSV) Write(r *ocr.Result) error {
	if !t.hdr {
		err := t.w.Write([]string{
			"timestamp",
			"expected",
			"result",
			"status",
		})
		if err != nil {
			return err
		}
		t.hdr = true
	}
	ts, err := r.TimeStamp.MarshalText()
	if err != nil {
		return err
	}
	return t.w.Write([]string{
		string(ts),
		r.Expected,
		r.Text,
		r.Status.String(),
	})
}

func (t ftCSV) Close() error {
	t.w.Flush()
	if err := t.w.Error(); err != nil {
		return err
	}
	return t.c.Close()
}

// newFTCSV creates a new CSV encoder.
func newFTCSV(wc io.WriteCloser) (WriteCloser, error) {
	return &ftCSV{
		w: csv.NewWriter(wc),
		c: wc,
	}, nil
}

// ftJSON writes JSON encoded output.
type ftJSON struct {
	w *json.Encoder
	c io.Closer
}

func (t ftJSON) Write(r *ocr.Result) error {
	return t.w.Encode(r)
}

func (t ftJSON) Close() error {
	return t.c.Close()
}

// newFTJSON creates a new JSON encoder.
func newFTJSON(wc io.WriteCloser) (WriteCloser, error) {
	return ftJSON{
		w: json.NewEncoder(wc),
		c: wc,
	}, nil
}
