//go:build embedalldotmat

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
		embed{
			Keyword: "dm-5x5",
			Ext:     ".traineddata",
			Bytes:   dm5x5,
		},
		embed{
			Keyword: "dm-display",
			Ext:     ".traineddata",
			Bytes:   dmDisplay,
		},
		embed{
			Keyword: "dm-dotrice",
			Ext:     ".traineddata",
			Bytes:   dmDotrice,
		},
		embed{
			Keyword: "dm-ft",
			Ext:     ".traineddata",
			Bytes:   dmFT,
		},
		embed{
			Keyword: "dm-lcd",
			Ext:     ".traineddata",
			Bytes:   dmLCD,
		},
		embed{
			Keyword: "dm-orario",
			Ext:     ".traineddata",
			Bytes:   dmOrario,
		},
		embed{
			Keyword: "dm-transit",
			Ext:     ".traineddata",
			Bytes:   dmTransit,
		},
	)
}

// dotmatrix is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed _internal/tesseract/dotmatrix/dotOCRDData1.traineddata
var dotmatrix []byte

// dm5x5 is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed _internal/tesseract/dotmatrix/5x5_Dots_FT_500.traineddata
var dm5x5 []byte

// dmDisplay is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed _internal/tesseract/dotmatrix/DisplayDots_FT_500.traineddata
var dmDisplay []byte

// dmDotrice is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed _internal/tesseract/dotmatrix/Dotrice_FT_500.traineddata
var dmDotrice []byte

// dmFT is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed _internal/tesseract/dotmatrix/DotMatrix_FT_500.traineddata
var dmFT []byte

// dmLCD is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed _internal/tesseract/dotmatrix/LCDDot_FT_500.traineddata
var dmLCD []byte

// dmOrario is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed _internal/tesseract/dotmatrix/Orario_FT_500.traineddata
var dmOrario []byte

// dmTransit is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed _internal/tesseract/dotmatrix/Transit_FT_500.traineddata
var dmTransit []byte
