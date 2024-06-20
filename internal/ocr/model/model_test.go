//go:build embedtess

package model

import (
	"os"
	"path"
	"testing"

	"github.com/stretchr/testify/assert"
)

type modelTest struct {
	Name    string
	Error   error
	IsEmbed bool
	Mod     Model
}

var modelTests = []modelTest{
	{
		Name:    "bad-empty",
		Error:   ErrBadModel,
		IsEmbed: false,
		Mod:     "",
	},
	{
		Name:    "good-file",
		Error:   nil,
		IsEmbed: false,
		Mod:     "dflt/eng.traineddata",
	},
	{
		Name:    "bad-missing-file",
		Error:   ErrBadModel,
		IsEmbed: false,
		Mod:     "definitely-does-not-exist.traineddata",
	},
	{
		Name:    "bad-file-name",
		Error:   ErrBadModel,
		IsEmbed: false,
		Mod:     "model.go",
	},
	{
		Name:    "good-embed",
		Error:   nil,
		IsEmbed: true,
		Mod:     "default",
	},
	{
		Name:    "bad-embed",
		Error:   ErrBadModel,
		IsEmbed: true,
		Mod:     "definitely-does-not-exist",
	},
}

// TestModelFile checks handling of model keywords and files.
func TestModelFile(t *testing.T) {
	for _, tt := range modelTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			m, rm, err := tt.Mod.File()

			if tt.Error == nil {
				// the returned model file name should always have
				// the proper extension
				assert.Equal(".traineddata", path.Ext(m))
				// if the model is externally supplied, the file should exist
				if !tt.IsEmbed {
					_, err := os.Stat(m)
					assert.Nil(err, "model file error")
				}
			}
			// check errors
			assert.Nil(rm(), "unexpected Remover error")
			assert.ErrorIs(err, tt.Error)
		})
	}
}
