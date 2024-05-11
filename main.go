package main

import (
	"encoding/json"
	"fmt"
	"log"
	"time"

	"github.com/Milover/ocr/internal/ocr"
)

const (
	config string = `
{
	"ocr": {
		"config_paths": [
			"internal/ocr/testdata/configs/test_neograf_5.patterns.config"
		],
		"model_dir_path": "internal/ocr/models/dotmatrix",
		"model": "dotOCRDData1"
	},
	"image_processing": {
		"preprocessing": [
			{
				"crop": {
					"left": -10,
					"right": 2000,
					"top": -10,
					"bottom": 1000
				}
			},
			{
				"rotate": {
					"angle": 360
				}
			},
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
					"value": 0.0,
					"max_value": 255.0,
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
			{
				"draw_text_boxes": {
				"thickness": 3
				}
			}
		]
	}
}
`
	testImg  string = "internal/ocr/testdata/images/neograf/imagefile_5.bmp"
	expected string = "V20000229"
)

func main() {
	start := time.Now()

	o := ocr.NewOCR()
	defer o.Delete()
	if err := json.Unmarshal([]byte(config), &o); err != nil {
		log.Fatal(err)
	}
	if err := o.Init(); err != nil {
		log.Fatal(err)
	}
	init := time.Now()

	if err := o.P.ReadImage(testImg, 0); err != nil {
		log.Fatal(err)
	}
	readimg := time.Now()

	if err := o.P.Preprocess(); err != nil {
		log.Fatal(err)
	}
	o.T.SetImage(*o.P, 1)
	preproc := time.Now()

	text, err := o.T.DetectAndRecognize()
	if err != nil {
		log.Fatal(err)
	}
	recog := time.Now()

	if err := o.P.Postprocess(*o.T); err != nil {
		log.Fatal(err)
	}
	end := time.Now()

	o.P.ShowImage("result")

	if text != expected {
		log.Fatalf("FAIL: got '%v', expected '%v'", text, expected)
	}
	fmt.Printf("got: %v\n", text)
	fmt.Printf("initialize     : %vms\n", init.Sub(start).Milliseconds())
	fmt.Printf("read image     : %vms\n", readimg.Sub(init).Milliseconds())
	fmt.Printf("preprocess     : %vms\n", preproc.Sub(readimg).Milliseconds())
	fmt.Printf("ocr            : %vms\n", recog.Sub(preproc).Milliseconds())
	fmt.Printf("postprocess    : %vms\n", end.Sub(recog).Milliseconds())
	fmt.Printf("total          : %vms\n", end.Sub(start).Milliseconds())
}
