//go:build embedtess

package model

import (
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

type modelTest struct {
	Name  string
	Error error
	Mod   Model
}

var modelTests = []modelTest{
	{
		Name:  "bad-empty",
		Error: ErrModel,
		Mod:   "",
	},
	{
		Name:  "bad-handle",
		Error: ErrModel,
		Mod:   "definitely-does-not-exist",
	},
	// FIXME: should we check file contents/extensions?
	//	{
	//		Name:    "bad-file-contents",
	//		Error:   ErrModel,
	//		Mod:     "model.go",
	//	},
	{
		Name:  "good-file",
		Error: nil,
		Mod:   "_internal/tesseract/dflt/eng.traineddata",
	},
	{
		Name:  "good-embed",
		Error: nil,
		Mod:   "default",
	},
}

// TestModelFile checks handling of model keywords and files.
func TestModelFile(t *testing.T) {
	for _, tt := range modelTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			f, cleanup, err := tt.Mod.File()
			assert.ErrorIs(err, tt.Error)

			// check the model file
			if tt.Error == nil {
				_, err = os.Stat(f)
				assert.Nil(err, "model file error")
			}

			// cleanup
			assert.Nil(cleanup(), "unexpected Cleanup error")
		})
	}
}
