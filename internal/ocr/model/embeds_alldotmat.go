//go:build embedalldotmat

package model

import (
	_ "embed"
)

// init populates 'embeds' with available embedded models.
func init() {
	embeds["dotmatrix"] = dotmatrix
	embeds["dm_5x5"] = dm5x5
	embeds["dm_display"] = dmDisplay
	embeds["dm_dotrice"] = dmDotrice
	embeds["dm_ft"] = dmFT
	embeds["dm_lcd"] = dmLCD
	embeds["dm_orario"] = dmOrario
	embeds["dm_transit"] = dmTransit
}

// dotmatrix is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed dotmatrix/dotOCRDData1.traineddata
var dotmatrix []byte

// dm5x5 is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed dotmatrix/5x5_Dots_FT_500.traineddata
var dm5x5 []byte

// dmDisplay is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed dotmatrix/DisplayDots_FT_500.traineddata
var dmDisplay []byte

// dmDotrice is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed dotmatrix/Dotrice_FT_500.traineddata
var dmDotrice []byte

// dmFT is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed dotmatrix/DotMatrix_FT_500.traineddata
var dmFT []byte

// dmLCD is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed dotmatrix/LCDDot_FT_500.traineddata
var dmLCD []byte

// dmOrario is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed dotmatrix/Orario_FT_500.traineddata
var dmOrario []byte

// dmTransit is a proprietary dot-matrix font model.
// For more info, see:
// https://github.com/ameera3/OCR_Expiration_Date
//
//go:embed dotmatrix/Transit_FT_500.traineddata
var dmTransit []byte
