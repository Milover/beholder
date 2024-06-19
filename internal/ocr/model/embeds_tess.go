//go:build embedtess

package model

import (
	_ "embed"
)

// init populates 'embeds' with available embedded models.
func init() {
	embeds["best"] = best
	embeds["default"] = dflt
	embeds["fast"] = fast
}

// best is the most accurate tesseract-provided model.
// For more info, see:
// https://github.com/tesseract-ocr/tessdata_best
//
//go:embed best/eng.traineddata
var best []byte

// dflt is the default tesseract-provided model.
// For more info, see:
// https://github.com/tesseract-ocr/tessdata
//
//go:embed dflt/eng.traineddata
var dflt []byte

// fast is a fast, integer version tesseract-provided model.
// For more info, see:
// https://github.com/tesseract-ocr/tessdata_fast
//
//go:embed fast/eng.traineddata
var fast []byte
