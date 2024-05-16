//go:build integration

package ocr

import (
	"encoding/json"
	"io"
	"os"
	"path"
	"testing"

	"github.com/stretchr/testify/assert"
)

type pipelineTest struct {
	Name     string
	Error    error
	Image    string
	ImageSet string
	Expected string
	Config   string
}

var pipelineTests = []pipelineTest{
	{
		Name:     "neograf",
		Error:    nil,
		ImageSet: "testdata/images/neograf",
		Image:    "imagefile_14.bmp",
		Expected: "V20000229",
		Config: `
{
	"ocr": {
		"config_paths": [
			"testdata/configs/test_neograf.patterns.config"
		],
		"model_dir_path": "_models/dotmatrix",
		"model": "dotOCRDData1",
		"page_seg_mode": "single_line",
		"variables": {
			"load_system_dawg": "0",
			"load_freq_dawg": "0",
			"tessedit_char_whitelist": "V0123456789"
		}
	},
	"image_processing": {
		"preprocessing": [
			{
				"auto_crop": {
					"kernel_size": 50,
					"text_width": 50,
					"text_height": 50,
					"padding": 10
				}
			},
			{
				"resize": {
					"width": 205,
					"height": 34
				}
			},
			{
				"normalize_brightness_contrast": {
					"clip_pct": 1.5
				}
			},
			{
				"gaussian_blur": {
					"kernel_width": 3,
					"kernel_height": 5,
					"sigma_x": 0,
					"sigma_y": 0
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
					"kernel_width": 4,
					"kernel_height": 4,
					"type": "open",
					"iterations": 1
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

// TestOCRRunOnce runs the OCR pipeline for a single image at a time and
// checks the results.
func TestOCRRunOnce(t *testing.T) {
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

			f, err := os.Open(path.Join(tt.ImageSet, tt.Image))
			defer f.Close()
			assert.Nil(err, "unexpected os.Open() error")
			buf, err := io.ReadAll(f)
			assert.Nil(err, "unexpected io.ReadAll() error")

			text, err := o.Run(buf)

			//t.Cleanup(func() {
			//	// do cleanup
			//})
			assert.Equal(tt.Expected, text)
			assert.Equal(tt.Error, err)
		})
	}
}

// TestOCRRunSet runs the OCR pipeline for a set of images and
// checks the results.
func TestOCRRunSet(t *testing.T) {
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

			dir, err := os.Open(tt.ImageSet)
			defer dir.Close()
			assert.Nil(err, "unexpected os.Open() error")
			files, err := dir.Readdirnames(-1)
			assert.Nil(err, "unexpected os.File.Readdirnames() error")

			for _, file := range files {
				f, err := os.Open(path.Join(tt.ImageSet, file))
				defer f.Close()
				assert.Nil(err, "unexpected os.Open() error")
				buf, err := io.ReadAll(f)
				assert.Nil(err, "unexpected io.ReadAll() error")

				text, err := o.Run(buf)

				assert.Equal(tt.Expected, text, f.Name())
				assert.Equal(tt.Error, err)
			}
		})
	}
}
