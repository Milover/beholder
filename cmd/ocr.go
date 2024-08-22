package cmd

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"os"
	"path"
	"runtime"
	"strings"
	"time"

	"github.com/Milover/beholder/internal/chrono"
	"github.com/Milover/beholder/internal/image"
	"github.com/Milover/beholder/internal/ocr"
	"github.com/Milover/beholder/internal/output"
	"github.com/Milover/beholder/internal/stopwatch"
	"github.com/spf13/cobra"
)

var (
	ocrCmd = &cobra.Command{
		Use:   "ocr [CONFIG] [FILE/DIRECTORY]",
		Short: "Run OCR pipeline from CONFIG on FILE or all files in DIRECTORY",
		Long:  `Run OCR pipeline from CONFIG on FILE or all files in DIRECTORY`,
		Args: cobra.MatchAll(
			cobra.ExactArgs(2),
		),
		RunE: runOCR,
	}
)

// OcrApp represents a program for running an OCR pipeline on an image or
// a set of images read from disc.
type OCRApp struct {
	T *ocr.Tesseract   `json:"tesseract"`
	P *image.Processor `json:"image_processing"`
	O *output.Output   `json:"output"`
}

// NewOCRApp creates a new OCR app.
func NewOCRApp() *OCRApp {
	return &OCRApp{
		T: ocr.NewTesseract(),
		P: image.NewProcessor(),
		O: output.NewOutput(),
	}
}

// Finalize releases resources held by the OCR app, flushes all buffer,
// closes all files and/or connections.
func (app *OCRApp) Finalize() error {
	app.T.Delete()
	app.P.Delete()
	return app.O.Close()
}

// Init initializes the OCR app by applying the configuration.
func (app *OCRApp) Init() error {
	if err := app.T.Init(); err != nil {
		return err
	}
	if err := app.P.Init(); err != nil {
		return err
	}
	if err := app.O.Init(); err != nil {
		return err
	}
	return nil
}

// Run is a function that runs the OCR pipeline for a single image file:
// reading, preprocessing, recognition and postprocessing.
func (app *OCRApp) Run(filename string, res *ocr.Result) error {
	sw := stopwatch.New()
	res.Reset()
	res.TimeStamp = sw.Start

	img, err := os.Open(filename)
	if err != nil {
		return err
	}
	defer img.Close()

	// run OCR
	// FIXME: this should probably happen in a different goroutine
	buf, err := io.ReadAll(img)
	if err != nil {
		return err
	}
	res.Timings.Set("read", sw.Lap())

	// FIXME: the read mode shouldn't be hardcoded
	if err := app.P.DecodeImage(buf, image.RMGrayscale); err != nil {
		return err
	}
	res.Timings.Set("decode", sw.Lap())

	if err := app.P.Preprocess(); err != nil {
		return err
	}
	if err := app.T.SetImage(app.P.GetRawImage(), 1); err != nil {
		return err
	}
	res.Timings.Set("preprocess", sw.Lap())

	if err := app.T.Recognize(res); err != nil {
		return err
	}
	res.Timings.Set("ocr", sw.Lap())

	if err := app.P.Postprocess(app.T.Ptr()); err != nil {
		return err
	}
	res.Timings.Set("postprocess", sw.Lap())

	// output results
	// FIXME: writes should happen in a different goroutine, since we don't
	// want the output to block pipeline execution
	if err := app.O.Write(res); err != nil {
		return err
	}
	// FIXME: this is only temporary, usually we don't want to flush after
	// each write, but it makes the output nicer
	if err := app.O.Flush(); err != nil {
		return err
	}
	res.Timings.Set("output", sw.Lap())

	res.Timings.Set("total", sw.Total())
	return nil
}

// Stats holds various program statistics.
type Stats struct {
	Result *ocr.Result
	// AvgTimings is a map of averaged [ocr.Result.Timings]
	AvgTimings chrono.Timings
	// InitDuration is the time elapsed while initializing the OCR pipeline.
	InitDuration time.Duration
	// ExecDuration is the total time elapsed while running the program.
	ExecDuration time.Duration
}

// NewStats creates a new ready to use Stats.
func NewStats() *Stats {
	return &Stats{
		Result: ocr.NewResult(),
	}
}

// Accumulate is a function which accumulates timings.
func (s *Stats) Accumulate(ts chrono.Timings) {
	if len(s.AvgTimings) == 0 {
		s.AvgTimings = make([]chrono.Timing, 0, len(ts))
		s.AvgTimings = append(s.AvgTimings, ts...)
	} else {
		for i := range ts {
			s.AvgTimings[i].Value += ts[i].Value
		}
	}
}

// Average is a function which computes timing averages for count timings.
func (s *Stats) Average(count int64) {
	for i := range s.AvgTimings {
		s.AvgTimings[i].Value /= time.Duration(count)
	}
}

func (s *Stats) String() string {
	var b strings.Builder
	fmt.Fprintf(&b, "%v\ninit: %v\nexec: %v\n",
		s.AvgTimings,
		s.InitDuration,
		s.ExecDuration,
	)
	return b.String()
}

func runOCR(cmd *cobra.Command, args []string) error {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	stats := NewStats()
	sw := stopwatch.New()

	// read config
	cfg, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}
	// setup OCR
	app := NewOCRApp()
	defer func() {
		if err := app.Finalize(); err != nil {
			log.Println("OCR app finalization error: ", err)
		}
	}()
	// unmarshall
	if err := json.Unmarshal(cfg, &app); err != nil {
		return err
	}
	if err := app.Init(); err != nil {
		return err
	}
	stats.InitDuration = sw.Lap()

	// process image(s)
	cliFile := args[1]
	info, err := os.Stat(cliFile)
	if err != nil {
		return err
	}
	count := int64(0) // No. results to average
	switch {
	case info.Mode().IsRegular():
		if err = app.Run(cliFile, stats.Result); err != nil {
			return err
		}
		stats.Accumulate(stats.Result.Timings)
		count++
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
		for _, filename := range filenames {
			if err := app.Run(path.Join(dir.Name(), filename), stats.Result); err != nil {
				return err
			}
			stats.Accumulate(stats.Result.Timings)
			count++
		}
	default:
		return fmt.Errorf("bad file: %v", info.Name())
	}
	stats.Average(count)
	stats.ExecDuration = sw.Total()

	//app.P.ShowImage("result")
	fmt.Println(stats)
	return nil
}
