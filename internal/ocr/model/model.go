package model

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path"

	"golang.org/x/exp/maps"
)

// embeds holds all embedded tesseract models (.traineddata files) available
// at runtime.
// The selection of models embedded depends on build tags used when building
// the program. Hence 'embeds' is populated at runtime, in init() functions
// in other files within this package, which define embedding rules based on
// build tags.
// TODO: replace individual, handwritten embed files with a single file
// generated at build time based on build tags.
var embeds = map[Model][]byte{}

// Available returns a list of embedded models available at runtime.
func Available() []Model {
	return maps.Keys(embeds)
}

// A set of errors which the package can return.
var (
	ErrBadModel = errors.New("bad OCR model")
)

// Model represents a keyword or a file name of a tesseract model (.traineddata
// file).
//
// A selection of models may be included (embedded) within the program binary
// which can be used by supplying the model keyword.
// An externally supplied model can also be used, in which case
// the model file name should be supplied.
type Model string

// Remover is a helper function which is used for clean up of model files.
type Remover func() error

// File returns the file name of the physical, on-disc model file, as well
// as a Remover used for model file clean up.
//
// The Remover should be called once the model file is no longer needed,
// i.e. once tesseract has loaded the model.
// Tesseract requires that models be supplied as physical files on disc,
// hence when using embedded models, a temporary model file is created which
// tesseract can use for loading the model.
// When an externally supplied model file is used, the Remover is a no-op.
//
// If m is not a valid embedded model keyword or a model file name,
// an ErrBadModel is returned.
func (m Model) File() (string, Remover, error) {
	rNoop := func() error {
		return nil
	}
	// boiler plate return in case of error when handling an embedded model
	retTmp := func(s string, err error) (string, Remover, error) {
		return "", rNoop, errors.Join(err, os.Remove(s))
	}
	// if m is an embedded model create a temporary file and return it
	if m.isValidEmbed() {
		b := embeds[m]
		f, err := os.CreateTemp("", "ocr_model_")
		if err != nil {
			return "", rNoop, err
		}
		// write the contents and close the file
		if _, err = f.Write(b); err != nil {
			return retTmp(f.Name(), err)
		}
		if err := f.Close(); err != nil {
			return retTmp(f.Name(), err)
		}
		// add the proper file extension so tesseract doesn't complain
		newname := f.Name() + ".traineddata"
		if err := os.Rename(f.Name(), newname); err != nil {
			// WARNING: we're assuming that if Rename fails, the original file
			// remains unchanged
			return retTmp(f.Name(), err)
		}
		return newname, func() error { return os.Remove(newname) }, err
	}
	// otherwise check if m is an externally supplied model
	if m.isValidFileName() {
		return string(m), rNoop, nil
	}
	return "", rNoop, fmt.Errorf("%w: %q", ErrBadModel, m)
}

// IsValid checks if m is a valid embedded model keyword or a model file name.
func (m Model) IsValid() error {
	if !m.isValidEmbed() && !m.isValidFileName() {
		return fmt.Errorf("%w: %q", ErrBadModel, m)
	}
	return nil
}

func (m Model) isValidEmbed() bool {
	_, ok := embeds[m]
	return ok
}

func (m Model) isValidFileName() bool {
	// for m to be a valid file name,
	// it must have the extension '.traineddata' (tesseract req.)
	if path.Ext(string(m)) != ".traineddata" {
		return false
	}
	// and the file must also exist
	if _, err := os.Stat(string(m)); err != nil {
		return false
	}
	return true
}

// UnmarshallJSON unmarshals m from JSON.
func (m *Model) UnmarshalJSON(data []byte) error {
	var s string
	if err := json.Unmarshal(data, &s); err != nil {
		return err
	}
	*m = Model(s)
	return nil
}

// MarshallJSON marshals m into JSON.
func (m Model) MarshalJSON() ([]byte, error) {
	return json.Marshal(string(m))
}
