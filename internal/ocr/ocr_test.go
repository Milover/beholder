//go:build integration

package ocr

import (
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
)

type pipelineTest struct {
	Name     string
	Error    error
	Image    string
	Expected string
	Config   string
}

var pipelineTests = []pipelineTest{
	{
		Name:     "neograf",
		Error:    nil,
		Image:    "testdata/images/neograf/imagefile_5.bmp",
		Expected: "V20000229",
		Config: `
{
	"ocr": {
		"config_paths": [
			"testdata/configs/test_neograf_5.patterns.config"
		],
		"model_dir_path": "_models/dotmatrix",
		"model": "dotOCRDData1"
	},
	"image_processing": {
		"preprocessing": [
			{
				"resize": {
					"width": 860,
					"height": 430
				}
			},
			{
				"normalize_brightness_contrast": {
					"clip_pct": 0.5
				}
			},
			{
				"median_blur": {
					"kernel_size": 3
				}
			},
			{
				"threshold": {
					"value": 0,
					"max_value": 255,
					"type": [ "binary", "otsu" ]
				}
			},
			{
				"morphology": {
					"kernel_type": "rectangle",
					"kernel_width": 3,
					"kernel_height": 3,
					"type": "open",
					"iterations": 5
				}
			}
		],
		"postprocessing": [
		]
	}
}
`,
	},
}

func TestOCRPipeline(t *testing.T) {
	for _, tt := range pipelineTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)
			var err error

			o := NewOCR()
			defer o.Delete()

			err = json.Unmarshal([]byte(tt.Config), &o)
			assert.Nil(err, "unexpected json.Unmarshal() error")

			err = o.Init()
			assert.Nil(err, "unexpected OCR.Init() error")

			err = o.P.ReadImage(tt.Image, 0)
			assert.Nil(err, "unexpected ImageProcessor.ReadImage() error")

			err = o.P.Preprocess()
			assert.Nil(err, "unexpected ImageProcessor.Preprocess() error")

			o.T.SetImage(*o.P, 1)

			text, err := o.T.DetectAndRecognize()
			assert.Nil(err, "unexpected Tesseract.DetectAndRecognize() error")

			err = o.P.Postprocess(*o.T)
			assert.Nil(err, "unexpected ImageProcessor.Postprocess() error")
			//o.P.ShowImage("result")

			//t.Cleanup(func() {
			//	// do cleanup
			//})
			assert.Equal(tt.Expected, text)
			//assert.Equal(tt.Error, nil)
		})
	}
}
