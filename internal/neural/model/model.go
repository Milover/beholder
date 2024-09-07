package model

import (
	"errors"
	"fmt"
	"os"
	"slices"
)

var (
	ErrModel = errors.New("bad model") // bad model handle (keyword or file path)
)

// Cleanup is a helper function which is used to clean up temporary model files.
type Cleanup func() error

// embed is an embedded NN model.
type embed struct {
	Keyword Model  // NN model keyword
	Ext     string // NN model file extension
	Bytes   []byte // raw bytes of the model definition, eg. weights
}

// embeds holds all embedded NN models available at runtime.
//
// The selection of models embedded depends on build tags used when building
// the program. Hence 'embeds' is populated at runtime, in init() functions
// in other files within this package, which define embedding rules based on
// build tags.
//
// TODO: replace individual, handwritten embed files with a single file
// generated at build time based on build tags.
var embeds []embed

// Available returns a list of embedded models available at runtime.
func Available() []Model {
	avail := make([]Model, 0, len(embeds))
	for i := range embeds {
		avail = append(avail, embeds[i].Keyword)
	}
	return avail
}

// getEmbedID returns the index within [embeds], of the first model whose
// [embed.Keyword] matches m, or -1 if none is found.
func getEmbedID(m Model) int {
	return slices.IndexFunc(embeds, func(e embed) bool {
		return e.Keyword == m
	})
}

// Model is a NN model definition handle.
//
// A selection of models may be included (embedded) within the program binary
// which can be used by setting Model to the model keyword.
// An externally supplied model can also be used, in which case
// Model is a file path.
type Model string

// File returns the file name of the on-disc NN model definition file, and
// a Cleanup used to clean up any temporary files.
//
// Cleanup should be called once the model file is no longer needed,
// i.e. once the model file has been loaded.
//
// The C-API requires that models be supplied as on-disc files, hence
// when using embedded models, a temporary file is created which the C-API
// uses to load the model.
// When an externally supplied model file is used, Cleanup is a no-op.
//
// If m is not a valid embedded model keyword or a valid file path,
// an ErrBadModel is returned.
func (m Model) File() (string, Cleanup, error) {
	rNoop := func() error {
		return nil
	}
	// boiler plate return in case of error when handling an embedded model
	retTmp := func(s string, err error) (string, Cleanup, error) {
		return "", rNoop, errors.Join(err, os.Remove(s))
	}
	// if m is an embedded model create a temporary file and return it
	if id := getEmbedID(m); id != -1 {
		f, err := os.CreateTemp("", "model_*"+embeds[id].Ext)
		if err != nil {
			return "", rNoop, err
		}
		// write the contents and close the file
		if _, err = f.Write(embeds[id].Bytes); err != nil {
			return retTmp(f.Name(), err)
		}
		if err := f.Close(); err != nil {
			return retTmp(f.Name(), err)
		}
		return f.Name(), func() error { return os.Remove(f.Name()) }, err
	}
	// otherwise check if m is an externally supplied model
	if m.isExternal() {
		return string(m), rNoop, nil
	}
	return "", rNoop, fmt.Errorf("%w: %q", ErrModel, m)
}

// isEmbed returns true if m is an (available) embedded model.
func (m Model) isEmbed() bool {
	return getEmbedID(m) != -1
}

// isExternal returns true if m refers to a vaild model file path.
//
// FIXME: file contents are not checked.
func (m Model) isExternal() bool {
	info, err := os.Stat(string(m))
	return err == nil && info.Mode().IsRegular()
}
