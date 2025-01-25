// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package output

import (
	"encoding/json"
	"errors"
	"io"
	"os"

	"github.com/Milover/beholder/internal/enumutils"
)

var (
	ErrBadOutputTarget       = errors.New("bad target type")
	ErrOutputTargetEmptyFile = errors.New("output target file name empty")
)

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
	factory, ok := outTargetFactoryMap[t]
	if !ok {
		return nil, ErrBadOutputTarget
	}
	return factory(m)
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
