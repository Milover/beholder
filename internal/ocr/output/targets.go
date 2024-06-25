package output

import (
	"encoding/json"
	"errors"
	"io"
	"os"

	"github.com/Milover/ocr/internal/enumutils"
)

var (
	ErrBadTarget = errors.New("bad target type")
	ErrEmptyFile = errors.New("target file name empty")
)

// TargetType represents available output locations.
type TargetType int

const (
	TTNone TargetType = iota
	TTStdout
	TTFile
)

var (
	targetTypeMap = map[TargetType]string{
		TTNone:   "none",
		TTStdout: "stdout",
		TTFile:   "file",
	}
	invTargetTypeMap = enumutils.Invert(targetTypeMap)
)

func (w *TargetType) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, w, invTargetTypeMap)
}

func (w TargetType) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(w, targetTypeMap)
}

// ttFactory takes a JSON formatted write target spec and
// creates a new io.WriteCloser according to the spec.
type ttFactory func(json.RawMessage) (io.WriteCloser, error)

// ttFactoryMap is maps write targets to factory functions.
var ttFactoryMap = map[TargetType]ttFactory{
	TTNone:   newTTNone,
	TTStdout: newTTStdout,
	TTFile:   newTTFile,
}

func newTarget(t TargetType, m json.RawMessage) (io.WriteCloser, error) {
	switch t {
	case TTNone:
		return newTTNone(m)
	case TTStdout:
		return newTTStdout(m)
	case TTFile:
		return newTTFile(m)
	}
	return nil, ErrBadTarget
}

// ttNone is a dummy output target.
type ttNone struct{}

func (t ttNone) Write(p []byte) (int, error) {
	return len(p), nil
}

func (t ttNone) Close() error {
	return nil
}

// newTTNone creates a new ttNone from a JSON formatted spec.
func newTTNone(json.RawMessage) (io.WriteCloser, error) {
	return ttNone{}, nil
}

// ttStdout uses stdout as the output target.
type ttStdout struct {
	w *os.File
}

func (t ttStdout) Write(p []byte) (int, error) {
	return t.w.Write(p)
}

func (t ttStdout) Close() error {
	return nil
}

// newTTStdout creates a new ttStdout from a JSON formatted spec.
func newTTStdout(json.RawMessage) (io.WriteCloser, error) {
	return ttStdout{w: os.Stdout}, nil
}

// ttFile uses a file as the output target.
type ttFile struct {
	// File is the file path to which the output is written.
	File string `json:"file"`

	f *os.File
}

func (t ttFile) Write(p []byte) (int, error) {
	return t.f.Write(p)
}

func (t ttFile) Close() error {
	return t.f.Close()
}

// newTTFile creates a new ttFile from a JSON formatted spec.
func newTTFile(m json.RawMessage) (io.WriteCloser, error) {
	var t ttFile
	var err error
	if err = json.Unmarshal(m, &t); err != nil {
		return t, err
	}
	if len(t.File) == 0 {
		return nil, ErrEmptyFile
	}
	// TODO: whould log warning or abort if the file exists
	if t.f, err = os.Create(t.File); err != nil {
		return t, err
	}
	return t, nil
}
