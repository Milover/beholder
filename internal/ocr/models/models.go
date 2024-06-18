//go:build release

package models

import _ "embed"

func init() {
}

var Models = map[string][]byte{
	"best":       best,
	"default":    dflt,
	"fast":       fast,
	"dotmatrix":  dotmatrix,
	"dm-5x5":     dm5x5,
	"dm-display": dmDisplay,
	"dm-dotrice": dmDotrice,
	"dm-ft":      dmFT,
	"dm-lcd":     dmLCD,
	"dm-orario":  dmOrario,
	"dm-transit": dmTransit,
}

//go:embed best/eng.traineddata
var best []byte

//go:embed dflt/eng.traineddata
var dflt []byte

//go:embed fast/eng.traineddata
var fast []byte

//go:embed dotmatrix/dotOCRDData1.traineddata
var dotmatrix []byte

//go:embed dotmatrix/5x5_Dots_FT_500.traineddata
var dm5x5 []byte

//go:embed dotmatrix/DisplayDots_FT_500.traineddata
var dmDisplay []byte

//go:embed dotmatrix/Dotrice_FT_500.traineddata
var dmDotrice []byte

//go:embed dotmatrix/DotMatrix_FT_500.traineddata
var dmFT []byte

//go:embed dotmatrix/LCDDot_FT_500.traineddata
var dmLCD []byte

//go:embed dotmatrix/Orario_FT_500.traineddata
var dmOrario []byte

//go:embed dotmatrix/Transit_FT_500.traineddata
var dmTransit []byte
