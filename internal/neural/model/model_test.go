// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package model

import (
	"os"
	"path"
	"testing"

	"github.com/stretchr/testify/assert"
)

const (
	// assetsDir is the relative path (w.r.s. to this file) to the directory
	// containing testing assets (models, images, etc.)
	assetsDir = "../../../test/assets"
)

// modelPath appends a model file name to the test assets directory.
func modelPath(name string) Model {
	return Model(path.Join(assetsDir, "models", name))
}

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
		Mod:   modelPath("tesseract-eng-fast.traineddata"),
	},
	// NOTE: we no longer ship models, so this is disabled until we decide
	// how we want to support embedding.
	//	{
	//		Name:  "good-embed",
	//		Error: nil,
	//		Mod:   "default",
	//	},
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
