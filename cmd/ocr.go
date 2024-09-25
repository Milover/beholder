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

	"github.com/Milover/beholder/internal/imgproc"
	"github.com/Milover/beholder/internal/models"
	"github.com/Milover/beholder/internal/neural"
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
	Y *neural.YOLOv8     `json:"yolov8"`
	T *neural.Tesseract  `json:"tesseract"`
	P *imgproc.Processor `json:"image_processing"`
	O *output.Output     `json:"output"`
	F Filename[id]       `json:"filename"`
}

// NewOCRApp creates a new OCR app.
func NewOCRApp() *OCRApp {
	return &OCRApp{
		Y: neural.NewYOLOv8(),
		T: neural.NewTesseract(),
		P: imgproc.NewProcessor(),
		O: output.NewOutput(),
		F: Filename[id]{
			FString: "img_%v.jpeg",
			Fields:  []string{"ID"},
		},
	}
}

// Finalize releases resources held by the OCR app, flushes all buffer,
// closes all files and/or connections.
func (app *OCRApp) Finalize() error {
	app.Y.Delete()
	app.T.Delete()
	app.P.Delete()
	return app.O.Close()
}

// Init initializes the OCR app by applying the configuration.
func (app *OCRApp) Init() error {
	if err := app.Y.Init(); err != nil {
		return err
	}
	if err := app.T.Init(); err != nil {
		return err
	}
	if err := app.P.Init(); err != nil {
		return err
	}
	if err := app.O.Init(); err != nil {
		return err
	}
	if err := app.F.Init(id{}); err != nil {
		return err
	}
	return nil
}

// Run is a function that runs the OCR pipeline for a single image file:
// reading, preprocessing, recognition and postprocessing.
func (app *OCRApp) Run(filename string, imgID id, res *models.Result) error {
	sw := stopwatch.New()
	res.Reset()
	res.Timestamp = sw.Start

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
	if err := app.P.DecodeImage(buf, imgproc.RMColor); err != nil {
		return err
	}
	res.Timings.Set("decode", sw.Lap())

	// detect
	if err := app.Y.Inference(app.P.GetRawImage(), res); err != nil {
		return err
	}
	res.Timings.Set("yolo", sw.Lap())

	// loop for each ROI
	tRes := models.NewResult()
	for i := range res.Boxes {
		app.P.SetROI(res.Boxes[i])
		if err := app.P.Preprocess(); err != nil {
			return err
		}
		//res.Timings.Set("preprocess", sw.Lap())
		if err := app.T.Inference(app.P.GetRawImage(), tRes); err != nil {
			return err
		}
		// adjust results
		res.Text[i] = strings.Join(tRes.Text, " ")
		for _, c := range tRes.Confidences {
			res.Confidences[i] *= c / 100.0
		}
		// XXX: remove
		if err := app.P.WriteImage(fmt.Sprintf("tmp_%v_%d.png", imgID.ID, i)); err != nil {
			return err
		}
		// XXX: remove
		app.P.ResetROI()
	}
	// end ROI loop
	res.Timings.Set("ocr", sw.Lap())

	if err := app.P.Postprocess(res); err != nil {
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
	var fname string
	if fname, err = app.F.Get(&imgID); err != nil {
		return err
	}
	if err := app.P.WriteImage(fname); err != nil {
		return err
	}
	res.Timings.Set("output", sw.Lap())

	res.Timings.Set("total", sw.Total())
	return nil
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
	switch {
	case info.Mode().IsRegular():
		id := id{ID: fmt.Sprintf("%07d", stats.avgCount)}
		if err = app.Run(cliFile, id, stats.Result); err != nil {
			return err
		}
		stats.RollingAverage(stats.Result.Timings)
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
			id := id{ID: fmt.Sprintf("%07d", stats.avgCount)}
			file := path.Join(dir.Name(), filename)

			log.Printf("processing (%d/%d): %v", stats.avgCount+1, len(filenames), file)
			if err := app.Run(file, id, stats.Result); err != nil {
				//return err
				log.Println("processing error:", err)
				continue
			}
			stats.RollingAverage(stats.Result.Timings)
		}
	default:
		return fmt.Errorf("bad file: %v", info.Name())
	}
	stats.ExecDuration = sw.Total()

	//app.P.ShowImage("result")
	fmt.Println(stats)
	return nil
}
