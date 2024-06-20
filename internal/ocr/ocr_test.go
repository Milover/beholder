//go:build integration

package ocr

import (
	"encoding/csv"
	"encoding/json"
	"fmt"
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
	Expected string
	Result   string
}

// Function buildTable builds the test failure table string.
func buildTable(failures []testFailure) string {
	var maxFile, maxExp, maxRes int
	for _, f := range failures {
		lFile := len(f.Filename)
		lExp := len(fmt.Sprintf("%q", f.Expected))
		lRes := len(fmt.Sprintf("%q", f.Result))

		if lFile > maxFile {
			maxFile = lFile
		}
		if lExp > maxExp {
			maxExp = lExp
		}
		if lRes > maxRes {
			maxRes = lRes
		}
	}
	format := fmt.Sprintf("| %%-%ds | %%-%dq | %%-%dq |\n",
		maxFile, maxExp, maxRes)
	var table string
	for _, f := range failures {
		table += fmt.Sprintf(format, f.Filename, f.Expected, f.Result)
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
		],
		"model": "model/dotmatrix/dotOCRDData1.traineddata",
		"page_seg_mode": "single_line",
		"variables": {
			"load_system_dawg": "0",
			"load_freq_dawg": "0",
			"tessedit_char_whitelist": "V0123456789"
		},
		"patterns": [
			"\\A\\d\\d\\d\\d\\d\\d\\d\\d"
		]
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
		],
		"model": "model/dotmatrix/Transit_FT_500.traineddata",
		"page_seg_mode": "single_block",
		"variables": {
			"load_system_dawg": "0",
			"load_freq_dawg": "0",
			"classify_bln_numeric_mode": "1",
			"tessedit_char_whitelist": ".:0123456789"
		},
		"patterns": [
			"\\d\\d.\\d\\d.\\d\\d\\d\\d.",
			"\\d\\d:\\d\\d"
		]
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

func setupOCRTest(pt pipelineTest) (*OCR, [][]string, error) {
	var records [][]string
	o := NewOCR()
	// unmarshal
	if err := json.Unmarshal([]byte(pt.Config), &o); err != nil {
		return o, records, fmt.Errorf("could not unmarshall JSON: %w", err)
	}
	// initialize
	if err := o.Init(); err != nil {
		return o, records, fmt.Errorf("could not initialize OCR: %w", err)
	}
	// read the expected values if necessary
	if len(pt.ExpectedFile) != 0 {
		f, err := os.Open(path.Join(pt.ImageSet, pt.ExpectedFile))
		if err != nil {
			return o, records, err
		}
		defer f.Close()
		r := csv.NewReader(f)
		r.Comma = '\t'
		if records, err = r.ReadAll(); err != nil {
			return o, records, fmt.Errorf("could not read 'expected' file: %w", err)
		}
	}
	return o, records, nil
}

// getExpected gets the expected result for an image.
// If working with an image set, it finds the result from 'records', otherwise
// it returns 'dflt'.
func getExpected(records [][]string, image string, dflt string) string {
	if len(records) == 0 {
		return dflt
	}
	var expected string
	for _, record := range records {
		if record[0] == image {
			expected = strings.ReplaceAll(record[1], "\\n", "\n")
			break
		}
	}
	return expected
}

// TestOCRRunOnce runs the OCR pipeline for a single image at a time and
// checks the results.
func TestOCRRunOnce(t *testing.T) {
	for _, tt := range pipelineTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			// setup
			o, expectedRecords, err := setupOCRTest(tt)
			defer o.Delete()
			assert.Nil(err, "unexpected OCR test setup error")

			// read image
			img, err := os.Open(path.Join(tt.ImageSet, tt.Image))
			assert.Nil(err, "could not open image")
			defer img.Close()

			// test
			res, err := o.Run(img)
			assert.Equal(tt.Error, err, "unexpected OCR error")

			// set expected
			expected := getExpected(expectedRecords, tt.Image, tt.Expected)
			assert.NotEqual(0, len(expected),
				fmt.Sprintf("could not find expected for %v", tt.Image))

			// check results
			assert.Equal(tt.Mapper(expected), tt.Mapper(res.Text), "OCR failure")
		})
	}
}

// TestOCRRunSet runs the OCR pipeline for a set of images and
// checks the results.
func TestOCRRunSet(t *testing.T) {
	for _, tt := range pipelineTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			// setup
			o, expectedRecords, err := setupOCRTest(tt)
			defer o.Delete()
			assert.Nil(err, "unexpected OCR test setup error")

			// get the image set file names
			dir, err := os.Open(tt.ImageSet)
			assert.Nil(err, "could not open image set directory")
			defer dir.Close()
			filenames, err := dir.Readdirnames(-1)
			assert.Nil(err, "could not read image set file names")

			// test each image
			var failures []testFailure
			var errFail error
			for _, filename := range filenames {
				// skip non-image files
				if !slices.Contains(imgFormats, path.Ext(filename)) {
					continue
				}
				// read image
				img, err := os.Open(path.Join(tt.ImageSet, filename))
				assert.Nil(err, "could not open image")
				defer img.Close()

				// test
				res, err := o.Run(img)
				assert.Equal(tt.Error, err, "unexpected OCR error")

				// set expected
				expected := getExpected(expectedRecords, filename, tt.Expected)
				assert.NotEqual(0, len(expected),
					fmt.Sprintf("could not find expected for %v", filename))

				// check results
				if tt.Mapper(res.Text) != tt.Mapper(expected) {
					errFail = fmt.Errorf("OCR failure")
					failures = append(
						failures,
						testFailure{
							Filename: img.Name(),
							Expected: expected,
							Result:   res.Text,
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
			msg := fmt.Sprintf("failed: %v/%v\n", len(failures), len(filenames))
			msg += buildTable(failures)

			assert.Nil(errFail, msg)
		})
	}
}

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
	"ocr": {
		"model": "model/dotmatrix/dotOCRDData1.traineddata",
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

// TestOCRPatternLoad checks if the pattern(s) from the config are properly
// loaded when OCR is initialized.
// NOTE: this is a stupid test, it doesn't test shit.
func TestOCRPatternLoad(t *testing.T) {
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
