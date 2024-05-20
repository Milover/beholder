//go:build integration

package ocr

import (
	"encoding/csv"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"path"
	"slices"
	"strings"
	"testing"

	"github.com/stretchr/testify/assert"
)

var imgFormats = []string{
	".bmp",
	".jpeg",
	".jpg",
}

type testFailure struct {
	Filename string
	Error    error
	Expected string
	Result   string
}

// Function calculateWidths calculates the maximum width of each column.
func calculateWidths(failures []testFailure) (int, int, int, int) {
	var maxFile, maxExp, maxRes, maxErr int

	for _, f := range failures {
		lFile := len(f.Filename)
		lExp := len(fmt.Sprintf("%q", f.Expected))
		lRes := len(fmt.Sprintf("%q", f.Result))
		lErr := 0

		if lFile > maxFile {
			maxFile = lFile
		}
		if lExp > maxExp {
			maxExp = lExp
		}
		if lRes > maxRes {
			maxRes = lRes
		}
		if f.Error != nil {
			if lErr = len(f.Error.Error()); lErr > maxErr {
				maxErr = lErr
			}
		}
	}
	return maxFile, maxExp, maxRes, maxErr
}

// Function buildTable builds the test failure table string.
func buildTable(failures []testFailure) string {
	maxFile, maxExp, maxRes, maxErr := calculateWidths(failures)

	format := fmt.Sprintf(
		"| %%-%ds | %%-%dq | %%-%dq | %%-%ds |\n",
		maxFile,
		maxExp,
		maxRes,
		maxErr,
	)
	var table string
	for _, f := range failures {
		errStr := ""
		if f.Error != nil {
			errStr = f.Error.Error()
		}
		table += fmt.Sprintf(format, f.Filename, f.Expected, f.Result, errStr)
	}
	return table
}

type pipelineTest struct {
	Name  string
	Error error
	// Image is the file name of an image.
	// Used in 'Test.*Once' tests.
	Image string
	// ImageSet is the directory path containing image files.
	// Used in 'Test.*Set' tests.
	ImageSet string
	// Expected is the image text (expected OCR result).
	Expected string
	// ExpectedFile is a CSV file containing image file names and
	// image text (expected OCR result).
	// It is only used if defined, and if it is defined, Expected is ignored.
	// If all images in a set have the same text use Expected.
	ExpectedFile string
	// MapFunc is used to relax the requirements of the expected text.
	// The OCR process can be allowed to missinterpret certain
	// non-critical characters, e.g. punctuation, without failing the test.
	Mapper func(string) string
	Config string
}

var pipelineTests = []pipelineTest{
	{
		Name:     "neograf",
		Error:    nil,
		ImageSet: "testdata/images/neograf",
		Image:    "imagefile_14.bmp",
		Expected: "V20000229",
		Mapper:   func(s string) string { return s },
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
					"kernel_size": 35,
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
				"threshold": {
					"value": 0,
					"max_value": 255,
					"type": [ "binary", "otsu" ]
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
				"equalize_histogram": null
			},
			{
				"morphology": {
					"kernel_type": "rectangle",
					"kernel_width": 3,
					"kernel_height": 3,
					"type": "dilate",
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
	{
		Name:         "dukat",
		Error:        nil,
		ImageSet:     "testdata/images/dukat",
		Image:        "imagefile_115.jpeg",
		ExpectedFile: "expected.csv",
		Mapper: func(s string) string {
			return strings.Map(
				func(r rune) rune {
					switch r {
					case '.', ',', ';', ':', ' ':
						return -1 // drop
					}
					return r
				}, s)
		},
		Config: `
{
	"ocr": {
		"config_paths": [
			"testdata/configs/test_dukat.patterns.config"
		],
		"model_dir_path": "_models/dotmatrix",
		"model": "Transit_FT_500",
		"page_seg_mode": "single_block",
		"variables": {
			"load_system_dawg": "0",
			"load_freq_dawg": "0",
			"classify_bln_numeric_mode": "1",
			"tessedit_char_whitelist": ".:0123456789"
		}
	},
	"image_processing": {
		"preprocessing": [
			{
				"crop": {
					"left": 700,
					"top": 225,
					"width": 650,
					"height": 800
				}
			},
			{
				"invert": null
			},
			{
				"median_blur": {
					"kernel_size": 7
				}
			},
			{
				"auto_crop": {
					"kernel_size": 50,
					"text_width": 50,
					"text_height": 50,
					"padding": 15
				}
			},
			{
				"resize": {
					"width": 225,
					"height": 90
				}
			},
			{
				"rotate": {
					"angle": 180
				}
			},
			{
				"div_gaussian_blur": {
					"scale_factor": 255,
					"sigma_x": 5,
					"sigma_y": 5,
					"kernel_width": 0,
					"kernel_height": 0
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
				"auto_crop": {
					"kernel_size": 15,
					"text_width": 20,
					"text_height": 10,
					"padding": 10
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

			// new OCR
			o := NewOCR()
			defer o.Delete()
			// unmarshal
			err = json.Unmarshal([]byte(tt.Config), &o)
			assert.Nil(err, "could not unmarshall JSON")
			// initialize
			err = o.Init()
			assert.Nil(err, "could not initialize OCR")
			// read the image
			f, err := os.Open(path.Join(tt.ImageSet, tt.Image))
			defer f.Close()
			assert.Nil(err, fmt.Sprintf("could not open %q", f.Name()))
			buf, err := io.ReadAll(f)
			assert.Nil(err, fmt.Sprintf("could not read %q", f.Name()))

			// test
			text, err := o.Run(buf)

			// check results and errors
			var expected string
			if len(tt.ExpectedFile) != 0 {
				csvFile, err := os.Open(path.Join(tt.ImageSet, tt.ExpectedFile))
				defer csvFile.Close()
				assert.Nil(err, fmt.Sprintf("could not open %q", csvFile.Name()))
				r := csv.NewReader(csvFile)
				r.Comma = '\t'

				records, err := r.ReadAll()
				assert.Nil(err, fmt.Sprintf("could not read %q", csvFile.Name()))

				// find the expected string
				for _, record := range records {
					if record[0] == tt.Image {
						expected = strings.ReplaceAll(record[1], "\\n", "\n")
						break
					}
				}
				assert.NotEqual(0, len(expected),
					fmt.Sprintf("could not find expected for %v", tt.Image))
			} else {
				expected = tt.Expected
			}
			assert.Equal(tt.Mapper(expected), tt.Mapper(text))
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

			// new OCR
			o := NewOCR()
			defer o.Delete()
			// unmarshall
			err = json.Unmarshal([]byte(tt.Config), &o)
			assert.Nil(err, "unexpected json.Unmarshal() error")
			// initialize
			err = o.Init()
			assert.Nil(err, "unexpected OCR.Init() error")
			// get the image set file names
			dir, err := os.Open(tt.ImageSet)
			defer dir.Close()
			assert.Nil(err, "unexpected os.Open() error")
			files, err := dir.Readdirnames(-1)
			assert.Nil(err, "unexpected os.File.Readdirnames() error")
			// read the expected values if necessary
			var expectedRecords [][]string
			if len(tt.ExpectedFile) != 0 {
				csvFile, err := os.Open(path.Join(tt.ImageSet, tt.ExpectedFile))
				defer csvFile.Close()
				assert.Nil(err, fmt.Sprintf("could not open %q", csvFile.Name()))
				r := csv.NewReader(csvFile)
				r.Comma = '\t'

				expectedRecords, err = r.ReadAll()
				assert.Nil(err, fmt.Sprintf("could not read %q", csvFile.Name()))
			}

			// test each image
			var failures []testFailure
			var errFail error
			for _, file := range files {
				// read the image
				f, err := os.Open(path.Join(tt.ImageSet, file))
				defer f.Close()
				assert.Nil(err, "unexpected os.Open() error")
				if !slices.Contains(imgFormats, path.Ext(file)) {
					continue
				}
				buf, err := io.ReadAll(f)
				assert.Nil(err, "unexpected io.ReadAll() error")

				// test
				text, err := o.Run(buf)

				// check results and errors
				var expected string
				if len(tt.ExpectedFile) != 0 {
					for _, record := range expectedRecords {
						if record[0] == file {
							expected = strings.ReplaceAll(record[1], "\\n", "\n")
							break
						}
					}
					assert.NotEqual(0, len(expected),
						fmt.Sprintf("could not find expected for %v", tt.Image))
				} else {
					expected = tt.Expected
				}
				if tt.Mapper(text) != tt.Mapper(expected) || !errors.Is(err, tt.Error) {
					errFail = fmt.Errorf("OCR failure")
					failures = append(
						failures,
						testFailure{
							Filename: f.Name(),
							Error:    err,
							Expected: expected,
							Result:   text,
						},
					)
				}
			}
			// build the error message
			slices.SortFunc(
				failures,
				func(a, b testFailure) int {
					return strings.Compare(a.Filename, b.Filename)
				},
			)
			msg := fmt.Sprintf("failed: %v/%v\n", len(failures), len(files))
			msg += buildTable(failures)

			assert.Nil(errFail, msg)
		})
	}
}
