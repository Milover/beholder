//go:build embeddotmat && !embedalldotmat

package model

import (
	_ "embed"
)

// init populates 'embeds' with available embedded models.
func init() {
	embeds = append(embeds,
		embed{
			Keyword: "dotmatrix",
			Ext:     ".traineddata",
			Bytes:   dotmatrix,
		},
	)
}

// dotmatrix is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed _internal/tesseract/dotmatrix/dotOCRDData1.traineddata
var dotmatrix []byte
