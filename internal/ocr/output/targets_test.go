package output

import (
	"encoding/json"
	"io"
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

type targetTest struct {
	Name         string
	ErrConstruct error
	ErrWrite     error
	ErrClose     error
	Typ          TargetType
	Spec         string
	// TypCheck asserts the correct type of the constructed io.WriteCloser.
	TypCheck func(io.WriteCloser)
	// Cleanup, if defined, will be registered as the test cleanup function
	// using t.Cleanup(f func()).
	Cleanup func(io.WriteCloser) func()
}

var targetTests = []targetTest{
	{
		Name:         "bad-target",
		ErrConstruct: ErrBadTarget,
		ErrWrite:     nil,
		ErrClose:     nil,
		Typ:          TargetType(-1),
		Spec:         `{}`,
	},
	{
		Name:         "good-none",
		ErrConstruct: nil,
		ErrWrite:     nil,
		ErrClose:     nil,
		Typ:          TTNone,
		Spec:         `{}`,
		TypCheck:     func(wc io.WriteCloser) { _ = wc.(ttNone) },
	},
	{
		Name:         "good-stdout",
		ErrConstruct: nil,
		ErrWrite:     nil,
		ErrClose:     nil,
		Typ:          TTStdout,
		Spec:         `{}`,
		TypCheck:     func(wc io.WriteCloser) { _ = wc.(ttStdout) },
	},
	{
		Name:         "good-file",
		ErrConstruct: nil,
		ErrWrite:     nil,
		ErrClose:     nil,
		Typ:          TTFile,
		Spec:         `{"file": "definitely_does_not_exist"}`,
		TypCheck:     func(wc io.WriteCloser) { _ = wc.(ttFile) },
		Cleanup: func(wc io.WriteCloser) func() {
			return func() {
				t := wc.(ttFile)
				if err := os.Remove(t.File); err != nil {
					panic(err)
				}
			}
		},
	},
	{
		Name:         "bad-empty-file",
		ErrConstruct: ErrEmptyFile,
		ErrWrite:     nil,
		ErrClose:     nil,
		Typ:          TTFile,
		Spec:         `{}`,
	},
}

// TestTarget tests if output targets are properly set up from a JSON formatted
// configuration, and if the output targets function correctly.
func TestTarget(t *testing.T) {
	for _, tt := range targetTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			var m json.RawMessage
			err := json.Unmarshal([]byte(tt.Spec), &m)
			assert.Nil(err, "could not unmarshal JSON spec")

			wc, err := newTarget(tt.Typ, m)
			// optionally register the Cleanup function
			if tt.Cleanup != nil {
				t.Cleanup(tt.Cleanup(wc))
			}
			assert.ErrorIs(err, tt.ErrConstruct)

			// do further tests if wc was succesfully constructed
			if tt.ErrConstruct == nil {
				// check the constructed type
				assert.NotNil(tt.TypCheck, "TypCheck not defined")
				tt.TypCheck(wc)

				data := []byte("test")

				// write data to target
				count, err := wc.Write(data)
				assert.Equal(len(data), count, "mismatch in written bytes")
				assert.ErrorIs(err, tt.ErrWrite)

				// close the target
				err = wc.Close()
				assert.ErrorIs(err, tt.ErrClose)
			}
		})
	}
}
