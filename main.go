package main

import (
	"fmt"
	"log"
	"time"

	"github.com/Milover/ocr/internal/ocr"
)

const (
	json string = `
{
	"config_paths": ["internal/ocr/testdata/configs/test_neograf_5.patterns.config"],
	"model_path": "internal/ocr/models/dotmatrix",
	"model": "dotOCRDData1"
}
`
	testImg  string = "internal/ocr/testdata/images/neograf/imagefile_5.bmp"
	expected string = "V20000229"
)

func main() {
	start := time.Now()

	cfg := ocr.NewConfig(json)
	if cfg == nil {
		log.Fatal("Could not parse config")
	}
	t := ocr.NewTesseract(*cfg)
	if t == nil {
		log.Fatal("Could not initialize tesseract")
	}

	init := time.Now()

	ip := ocr.NewImageProcessor()
	if err := ip.ReadImage(testImg, 0); err != nil {
		log.Fatal(err)
	}

	readimg := time.Now()

	if err := ip.Preprocess(*cfg); err != nil {
		log.Fatal(err)
	}
	t.SetImage(*ip, 1)

	preproc := time.Now()

	if err := t.DetectText(*ip, *cfg); err != nil {
		log.Fatal(err)
	}
	text := t.RecognizeText()

	end := time.Now()

	if text != expected {
		log.Fatalf("FAIL: got '%v', expected '%v'", text, expected)
	}
	fmt.Printf("got: %v\n", text)
	fmt.Printf("initialize     : %vms\n", init.Sub(start).Milliseconds())
	fmt.Printf("read image     : %vms\n", readimg.Sub(init).Milliseconds())
	fmt.Printf("preprocess     : %vms\n", preproc.Sub(readimg).Milliseconds())
	fmt.Printf("ocr            : %vms\n", end.Sub(preproc).Milliseconds())
	fmt.Printf("total          : %vms\n", end.Sub(start).Milliseconds())
}
