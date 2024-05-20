package cmd

import (
	"encoding/json"
	"log"
	"os"
	"time"

	"github.com/Milover/ocr/internal/ocr"
	"github.com/spf13/cobra"
)

func run(cmd *cobra.Command, args []string) error {
	start := time.Now()

	// read config
	cfg, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}
	// new OCR
	o := ocr.NewOCR()
	defer o.Delete()
	// unmarshall
	if err := json.Unmarshal(cfg, &o); err != nil {
		return err
	}
	if err := o.Init(); err != nil {
		return err
	}
	init := time.Now()

	// read image
	// FIXME: image is cmdline supplied, and only 1 image/run
	img, err := os.ReadFile(args[1])
	if err != nil {
		return err
	}
	readimg := time.Now()

	text, err := o.Run(img)
	end := time.Now()

	//o.P.ShowImage("result")
	log.Printf("OCR result: %q", text)
	log.Printf("initialization : %vms\n", init.Sub(start).Milliseconds())
	log.Printf("image read     : %vms\n", readimg.Sub(init).Milliseconds())
	log.Printf("processing     : %vms\n", end.Sub(readimg).Milliseconds())
	log.Printf("total          : %vms\n", end.Sub(start).Milliseconds())
	return nil
}
