package cmd

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"path"
	"time"

	"github.com/Milover/ocr/internal/ocr"
	"github.com/Milover/ocr/internal/stopwatch"
	"github.com/spf13/cobra"
)

var (
	runCmd = &cobra.Command{
		Use:   "run [CONFIG] [FILE/DIRECTORY]",
		Short: "Run OCR pipeline from CONFIG on FILE or all files in DIRECTORY",
		Long:  `Run OCR pipeline from CONFIG on FILE or all files in DIRECTORY`,
		Args: cobra.MatchAll(
			cobra.ExactArgs(2),
		),
		RunE: run,
	}
)

// Stats holds various program statistics.
type Stats struct {
	OCRResult ocr.Result
	// InitDuration is the time elapsed while initializing the OCR pipeline.
	InitDuration time.Duration
	// ReadDuration is the time elapsed while reading the image.
	ReadDuration time.Duration
	// ExecDuration is the total time elapsed while running the program.
	ExecDuration time.Duration
}

func run(cmd *cobra.Command, args []string) error {
	var stats Stats
	sw := stopwatch.New()

	// read config
	cfg, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}
	// setup OCR
	o := ocr.NewOCR()
	defer o.Delete()
	// unmarshall
	if err := json.Unmarshal(cfg, &o); err != nil {
		return err
	}
	if err := o.Init(); err != nil {
		return err
	}
	stats.InitDuration = sw.Lap()

	// process image(s)
	cliFile := args[1]
	info, err := os.Stat(cliFile)
	if err != nil {
		return err
	}
	switch {
	case info.Mode().IsRegular():
		if stats.OCRResult, err = runOCR(cliFile, o); err != nil {
			return err
		}
	case info.Mode().IsDir():
		dir, err := os.Open(cliFile)
		if err != nil {
			return err
		}
		defer dir.Close()
		// OPTIMIZE: should maybe walk instead
		filenames, err := dir.Readdirnames(-1)
		if err != nil {
			return err
		}
		results := make([]ocr.Result, len(filenames))
		for _, filename := range filenames {
			res, err := runOCR(path.Join(dir.Name(), filename), o)
			if err != nil {
				return err
			}
			results = append(results, res)
		}
		stats.OCRResult = averageOCRTimings(results)
	default:
		return fmt.Errorf("bad file: %v", info.Name())
	}
	stats.ExecDuration = sw.Total()

	//o.P.ShowImage("result")
	log.Printf("initialize  : %v\n", stats.InitDuration.String())
	log.Printf("read        : %v\n", stats.OCRResult.ReadDuration.String())
	log.Printf("decode      : %v\n", stats.OCRResult.DecodeDuration.String())
	log.Printf("preprocess  : %v\n", stats.OCRResult.PreprocDuration.String())
	log.Printf("ocr         : %v\n", stats.OCRResult.OCRDuration.String())
	log.Printf("postprocess : %v\n", stats.OCRResult.PostprocDuration.String())
	log.Printf("total       : %v\n", stats.OCRResult.RunDuration.String())
	log.Printf("execution   : %v\n", stats.ExecDuration.String())
	return nil
}

// runOCR runs the OCR pipeline on a single image file.
func runOCR(filename string, o *ocr.OCR) (ocr.Result, error) {
	img, err := os.Open(filename)
	if err != nil {
		return ocr.Result{}, err
	}
	defer img.Close()

	res, err := o.Run(img)
	if err != nil {
		return res, err
	}
	log.Printf("OCR: %q", res.Text)

	return res, nil
}

// averageOCRTimings computes average OCR operation timings from the provided
// slice of OCR results.
func averageOCRTimings(rs []ocr.Result) ocr.Result {
	var res ocr.Result
	for _, r := range rs {
		res.RunDuration += r.RunDuration
		res.ReadDuration += r.ReadDuration
		res.DecodeDuration += r.DecodeDuration
		res.PreprocDuration += r.PreprocDuration
		res.OCRDuration += r.OCRDuration
		res.PostprocDuration += r.PostprocDuration
	}
	res.RunDuration /= time.Duration(len(rs))
	res.ReadDuration /= time.Duration(len(rs))
	res.DecodeDuration /= time.Duration(len(rs))
	res.PreprocDuration /= time.Duration(len(rs))
	res.OCRDuration /= time.Duration(len(rs))
	res.PostprocDuration /= time.Duration(len(rs))

	return res
}
