package neural

import (
	"io"
	"math"
	"os"
	"slices"
	"testing"

	"github.com/Milover/beholder/internal/imgproc"
	"github.com/Milover/beholder/internal/models"
	"github.com/stretchr/testify/assert"
)

func dfltTesseract() Network {
	n := NewTesseract()
	n.Model = "model/_internal/tesseract/dflt/eng.traineddata"
	return n
}
func dfltCRAFT() Network {
	n := NewCRAFT()
	n.Model = "model/_internal/craft/craft_320px.onnx"
	n.Config.Size = [2]int{320, 320}
	return n
}
func dfltEAST() Network {
	n := NewEAST()
	n.Model = "model/_internal/east/east.pb"
	n.Config.Size = [2]int{160, 160}
	return n
}
func dfltYOLOv8() Network {
	n := NewYOLOv8()
	n.Model = "model/_internal/yolo/yolov8n.onnx"
	return n
}

type networkTest struct {
	Name     string        // the name of the test
	Error    error         // expected error
	Image    string        // test image file path
	Expected models.Result // expected output
	Factory  Factory       // a factory for the Network used in the test
	Config   string        // JSON formatted Network config
}

var networkTests = []networkTest{
	// test tesseract
	{
		Name:    "tesseract-text-1-line-1-word",
		Error:   nil,
		Factory: dfltTesseract,
		Config:  "",
		Image:   "testdata/images/text_1_line_1_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 25, Top: 20, Right: 140, Bottom: 65},
			},
			Text:        []string{"TEXT"},
			Confidences: make([]float64, 1),
		},
	},
	{
		Name:    "tesseract-text-1-line-2-word",
		Error:   nil,
		Factory: dfltTesseract,
		Config:  "",
		Image:   "testdata/images/text_1_line_2_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 10, Top: 20, Right: 230, Bottom: 65},
			},
			Text:        []string{"TEXT TEXT"},
			Confidences: make([]float64, 1),
		},
	},
	{
		Name:    "tesseract-text-2-line-1-word",
		Error:   nil,
		Factory: dfltTesseract,
		Config:  "",
		Image:   "testdata/images/text_2_line_1_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 25, Top: 25, Right: 135, Bottom: 70},
				models.Rectangle{Left: 25, Top: 70, Right: 135, Bottom: 115},
			},
			Text:        []string{"TEXT", "TEXT"},
			Confidences: make([]float64, 2),
		},
	},
	{
		Name:    "tesseract-text-2-line-2-word",
		Error:   nil,
		Factory: dfltTesseract,
		Config:  "",
		Image:   "testdata/images/text_2_line_2_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 30, Top: 20, Right: 250, Bottom: 60},
				models.Rectangle{Left: 30, Top: 65, Right: 250, Bottom: 105},
			},
			Text:        []string{"TEXT TEXT", "TEXT TEXT"},
			Confidences: make([]float64, 2),
		},
	},
	// test craft
	{
		Name:    "craft-text-1-line-1-word",
		Error:   nil,
		Factory: dfltCRAFT,
		Config:  "",
		Image:   "testdata/images/text_1_line_1_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 25, Top: 20, Right: 140, Bottom: 65},
			},
			Text:        make([]string, 1),
			Confidences: make([]float64, 1),
		},
	},
	{
		Name:    "craft-text-1-line-2-word",
		Error:   nil,
		Factory: dfltCRAFT,
		Config:  "",
		Image:   "testdata/images/text_1_line_2_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 10, Top: 20, Right: 120, Bottom: 65},
				models.Rectangle{Left: 120, Top: 20, Right: 230, Bottom: 65},
			},
			Text:        make([]string, 2),
			Confidences: make([]float64, 2),
		},
	},
	{
		Name:    "craft-text-2-line-1-word",
		Error:   nil,
		Factory: dfltCRAFT,
		Config:  "",
		Image:   "testdata/images/text_2_line_1_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 25, Top: 25, Right: 135, Bottom: 70},
				models.Rectangle{Left: 25, Top: 70, Right: 135, Bottom: 115},
			},
			Text:        make([]string, 2),
			Confidences: make([]float64, 2),
		},
	},
	{
		Name:    "craft-text-2-line-2-word",
		Error:   nil,
		Factory: dfltCRAFT,
		Config:  "",
		Image:   "testdata/images/text_2_line_2_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 30, Top: 20, Right: 140, Bottom: 60},
				models.Rectangle{Left: 140, Top: 20, Right: 250, Bottom: 60},
				models.Rectangle{Left: 30, Top: 65, Right: 140, Bottom: 105},
				models.Rectangle{Left: 140, Top: 65, Right: 250, Bottom: 105},
			},
			Text:        make([]string, 4),
			Confidences: make([]float64, 4),
		},
	},
	// test east
	{
		Name:    "east-text-1-line-1-word",
		Error:   nil,
		Factory: dfltEAST,
		Config:  "",
		Image:   "testdata/images/text_1_line_1_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 25, Top: 20, Right: 140, Bottom: 65},
			},
			Text:        make([]string, 1),
			Confidences: make([]float64, 1),
		},
	},
	/* FIXME: disabled until we fix blob-to-image bounding box mapping
	{
		Name:    "east-text-1-line-2-word",
		Error:   nil,
		Factory: dfltEAST,
		Config:  "",
		Image:   "testdata/images/text_1_line_2_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 10, Top: 20, Right: 120, Bottom: 65},
				models.Rectangle{Left: 120, Top: 20, Right: 230, Bottom: 65},
			},
			Text:        make([]string, 2),
			Confidences: make([]float64, 2),
		},
	},
	{
		Name:    "east-text-2-line-1-word",
		Error:   nil,
		Factory: dfltEAST,
		Config:  "",
		Image:   "testdata/images/text_2_line_1_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 25, Top: 25, Right: 135, Bottom: 70},
				models.Rectangle{Left: 25, Top: 70, Right: 135, Bottom: 115},
			},
			Text:        make([]string, 2),
			Confidences: make([]float64, 2),
		},
	},
	{
		Name:    "east-text-2-line-2-word",
		Error:   nil,
		Factory: dfltEAST,
		Config:  "",
		Image:   "testdata/images/text_2_line_2_word.png",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 30, Top: 20, Right: 140, Bottom: 60},
				models.Rectangle{Left: 140, Top: 20, Right: 250, Bottom: 60},
				models.Rectangle{Left: 30, Top: 65, Right: 140, Bottom: 105},
				models.Rectangle{Left: 140, Top: 65, Right: 250, Bottom: 105},
			},
			Text:        make([]string, 4),
			Confidences: make([]float64, 4),
		},
	},
	*/
	// test yolov8
	{
		Name:    "yolov8-zidane",
		Error:   nil,
		Factory: dfltYOLOv8,
		Config:  "",
		Image:   "testdata/images/ultralytics_zidane.jpg",
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 90, Top: 170, Right: 1140, Bottom: 735},
				models.Rectangle{Left: 730, Top: 20, Right: 1160, Bottom: 735},
			},
			Text:        []string{"0", "0"}, // COCO "person" class
			Confidences: make([]float64, 2),
		},
	},
	// test unmarshalling
}

const netInfRepeat = 3 // No. inferencing re-runs during tests

// boxesInExpected reports whether all actual boxes are in the expected boxes.
//
// Since the element order might vary, both expected and actual are sorted
// in ascending order, based on the distance of the top left vertex from origin.
//
// Before sorting and comparing, the boxes are rotated by 90°/270°,
// if necessary (the result contains an angle), because some [Networks] yield
// rotated/transposed results, e.g. [neural.CRAFT].
func boxesInExpected(expected, actual *models.Result, t *testing.T) bool {
	if len(expected.Boxes) != len(actual.Boxes) {
		return false
	}
	// rotate by 90° increments only
	// TODO: handle arbitrary angles
	rot := func(rs []models.Rectangle, as []float64) {
		if len(rs) != len(as) {
			return
		}
		for i := range rs {
			switch math.Abs(math.Round(as[i])) { // FIXME: wtf, bro
			case 90.0, 270.0:
				rs[i].Rotate90()
			}
		}
	}
	rot(expected.Boxes, expected.Angles)
	rot(actual.Boxes, actual.Angles)

	// sort
	cmp := func(a, b models.Rectangle) int {
		da := math.Hypot(float64(a.Left), float64(a.Top))
		db := math.Hypot(float64(b.Left), float64(b.Top))
		switch {
		case da < db:
			return -1
		case da > db:
			return 1
		default:
			return 0
		}
	}
	slices.SortFunc(expected.Boxes, cmp)
	slices.SortFunc(actual.Boxes, cmp)

	ok := true
	for i := range expected.Boxes {
		//t.Logf("expected: %v\tactual: %v", expected[i], actual[i])
		ok = ok && actual.Boxes[i].In(expected.Boxes[i])
	}
	return ok
}

// TestNetworkInference repeatedly performs the inferencing step on an image and
// checks the results.
//
// TODO: explain what's a test-pass
func TestNetworkInference(t *testing.T) {
	for _, tt := range networkTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			// read image
			f, err := os.Open(tt.Image)
			assert.Nil(err, "could not open image file")
			defer f.Close()
			buf, err := io.ReadAll(f)
			assert.Nil(err, "could not read image file")

			// TODO: would be nice if we didn't have to import imgproc
			p := imgproc.NewProcessor()
			assert.Nil(p.Init(), "could not initialize image processor")
			defer p.Delete()
			assert.Nil(p.DecodeImage(buf, imgproc.RMColor), "could not decode image")
			img := p.GetRawImage()

			// set up network
			net := tt.Factory()
			defer net.Delete()
			assert.Nil(net.Init(), "unexpected Network.Init error")

			// test
			for _ = range netInfRepeat {
				// create a fresh result
				res := models.NewResult()
				assert.NotNil(res, "unexpected models.NewResult error")

				// inference
				err := net.Inference(img, res)
				assert.Equal(tt.Error, err, "unexpected Network.Inference error")

				// check text
				assert.ElementsMatch(tt.Expected.Text, res.Text,
					"text mismatch")
				// check boxes
				assert.Equal(len(tt.Expected.Boxes), len(res.Boxes),
					"boxes length mismatch")
				assert.True(boxesInExpected(&tt.Expected, res, t),
					"box overlap mismatch")
				// check confidences; XXX: checking length only
				assert.Equal(len(tt.Expected.Confidences), len(res.Confidences),
					"confidences length mismatch")
			}
		})
	}
}

/* TODO: fold this into the JSON unmarshalling test
type patternTest struct {
	Name     string
	Expected []string
	Error    error
	Config   string
}

var patternTests = []patternTest{
	{
		Name: "basic-good",
		Expected: []string{
			`\A\d\d`,
			`\A\d\d`,
		},
		Error: nil,
		Config: `
{
	"tesseract": {
		"model": "model/_internal/tesseract/dotmatrix/dotOCRDData1.traineddata",
		"variables": {
			"load_system_dawg": "0",
			"load_freq_dawg": "0"
		},
		"patterns": [
			"\\A\\d\\d",
			"\\A\\d\\d"
		]
	}
}
`,
	},
}

// TestTesseractPatternLoad checks if the pattern(s) from the config are
// properly loaded when Tesseract is initialized.
// FIXME: this is a stupid test, it doesn't test shit.
func TestTesseractPatternLoad(t *testing.T) {
	for _, tt := range patternTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			o := NewOCR()
			err := json.Unmarshal([]byte(tt.Config), &o)
			assert.Nil(err, "could not unmarshall JSON")

			// We're not sure the JSON will be unmarshalled, and
			// since we have no good way of checking the temporary file
			// contents (we don't want to store the file name) or what
			// Tesseract actually reads, we check what was unmarshalled here
			// since that's what will be the contents of the temporary file.
			assert.ElementsMatch(tt.Expected, o.T.Patterns, "bad OCR pattern(s)")

			// test
			err = o.Init()

			// check if patterns were properly loaded
			assert.Equal(tt.Error, err, "OCR init failure")
		})
	}
}
*/
