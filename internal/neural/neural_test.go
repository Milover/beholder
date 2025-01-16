// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package neural

import (
	"io"
	"math"
	"os"
	"path"
	"slices"
	"testing"

	"github.com/Milover/beholder/internal/imgproc"
	"github.com/Milover/beholder/internal/models"
	"github.com/Milover/beholder/internal/neural/model"
	"github.com/stretchr/testify/assert"
)

const (
	// assetsDir is the relative path (w.r.s. to this file) to the directory
	// containing testing assets (models, images, etc.)
	assetsDir = "../../test/assets"
)

// modelPath appends a model file name to the test assets directory.
func modelPath(name string) model.Model {
	return model.Model(path.Join(assetsDir, "models", name))
}

// imagePath appends an image file name to the test assets directory.
func imagePath(name string) string {
	return path.Join(assetsDir, "images", name)
}

func dfltTesseract() Network {
	n := NewTesseract()
	n.Model = modelPath("tesseract-eng-fast.traineddata")
	return n
}
func dfltCRAFT() Network {
	n := NewCRAFT()
	n.Model = modelPath("craft-320px.onnx")
	n.Config.Size = [2]int{320, 320}
	return n
}
func dfltEAST() Network {
	n := NewEAST()
	n.Model = modelPath("east.pb")
	n.Config.Size = [2]int{320, 320}
	return n
}
func dfltPARSeq() Network {
	n := NewPARSeq()
	n.Model = modelPath("parseq-128x32px.onnx")
	n.Config.Size = [2]int{128, 32}
	return n
}
func dfltYOLOv8() Network {
	n := NewYOLOv8()
	n.Backend = BackendCUDA
	n.Target = TargetCUDA
	n.Model = modelPath("yolov8n.onnx")
	n.Config.Size = [2]int{640, 640}
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
	{
		Name:    "tesseract-test-30px-640x640",
		Error:   nil,
		Factory: dfltTesseract,
		Config:  "",
		Image:   imagePath("test_30px_640x640.png"),
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 270, Top: 300, Right: 375, Bottom: 340},
			},
			Text:        []string{"TEST"},
			Confidences: make([]float64, 1),
		},
	},
	{
		Name:    "craft-test-30px-640x640",
		Error:   nil,
		Factory: dfltCRAFT,
		Config:  "",
		Image:   imagePath("test_30px_640x640.png"),
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 270, Top: 300, Right: 375, Bottom: 340},
			},
			Text:        make([]string, 1), // FIXME: shouldn't be necessary
			Confidences: make([]float64, 1),
		},
	},
	{
		Name:    "east-test-30px-640x640",
		Error:   nil,
		Factory: dfltEAST,
		Config:  "",
		Image:   imagePath("test_30px_640x640.png"),
		Expected: models.Result{
			Boxes: []models.Rectangle{
				models.Rectangle{Left: 270, Top: 300, Right: 375, Bottom: 340},
			},
			Text:        make([]string, 1),
			Confidences: make([]float64, 1),
		},
	},
	{
		Name:    "parseq-test-30px-128x32",
		Error:   nil,
		Factory: dfltPARSeq,
		Config:  "",
		Image:   imagePath("test_30px_128x32.png"),
		Expected: models.Result{
			Boxes: []models.Rectangle{ // FIXME: dummy value, shouldn't be necessary
				models.Rectangle{Left: -1, Top: -1, Right: 1, Bottom: 1},
			},
			Text:        []string{"TEST"},
			Confidences: make([]float64, 1),
		},
	},
	{
		Name:    "yolov8-zidane",
		Error:   nil,
		Factory: dfltYOLOv8,
		Config:  "",
		Image:   imagePath("ultralytics_zidane.jpg"),
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
func boxesInExpected(expected, actual *models.Result, _ *testing.T) bool {
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
		//t.Logf("expected: %v\tactual: %v", expected.Boxes[i], actual.Boxes[i])
		//t.Logf("expected: %v\tactual: %v", expected.Text[i], actual.Text[i])
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
			for range netInfRepeat {
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
