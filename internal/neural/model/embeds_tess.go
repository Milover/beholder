// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

//go:build embedtess

package model

import (
	_ "embed"
)

// init populates 'embeds' with available embedded models.
func init() {
	embeds = append(embeds,
		embed{
			Keyword: "best",
			Ext:     ".traineddata",
			Bytes:   best,
		},
		embed{
			Keyword: "default",
			Ext:     ".traineddata",
			Bytes:   dflt,
		},
		embed{
			Keyword: "fast",
			Ext:     ".traineddata",
			Bytes:   fast,
		},
	)
}

// best is the most accurate tesseract-provided model.
// For more info, see:
// https://github.com/tesseract-ocr/tessdata_best
//
//go:embed _internal/tesseract/best/eng.traineddata
var best []byte

// dflt is the default tesseract-provided model.
// For more info, see:
// https://github.com/tesseract-ocr/tessdata
//
//go:embed _internal/tesseract/dflt/eng.traineddata
var dflt []byte

// fast is a fast, integer version tesseract-provided model.
// For more info, see:
// https://github.com/tesseract-ocr/tessdata_fast
//
//go:embed _internal/tesseract/fast/eng.traineddata
var fast []byte
