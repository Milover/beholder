package cmd

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"runtime"
	"strings"
	"time"

	"github.com/Milover/beholder/internal/camera"
	"github.com/Milover/beholder/internal/imgproc"
	"github.com/Milover/beholder/internal/models"
	"github.com/Milover/beholder/internal/neural"
	"github.com/Milover/beholder/internal/output"
	"github.com/Milover/beholder/internal/stopwatch"
	"github.com/spf13/cobra"
)

var (
	demoCmd = &cobra.Command{
		Use:   "demo [CONFIG]",
		Short: "Run obj. detection/OCR using CONFIG to configure the program",
		Long:  `Run obj. detection/OCR using CONFIG to configure the program`,
		Args: cobra.MatchAll(
			cobra.ExactArgs(1),
		),
		RunE: runDemo,
	}
)

// DemoApp is a program for showcasing image acquisition, processing and
// web serving.
type DemoApp struct {
	Cs camera.Array           `json:"cameras"`
	Y  *neural.YOLOv8         `json:"yolov8"`
	T  *neural.Tesseract      `json:"tesseract"`
	P  *imgproc.Processor     `json:"image_processing"`
	O  *output.Output         `json:"output"`
	F  Filename[models.Image] `json:"filename"`

	TstImg string `json:"tst_camera_test_image"`
}

// NewDemoApp creates a new demo app.
func NewDemoApp() *DemoApp {
	return &DemoApp{
		Y: neural.NewYOLOv8(),
		T: neural.NewTesseract(),
		P: imgproc.NewProcessor(),
		O: output.NewOutput(),
		F: Filename[models.Image]{
			FString: "img_%v_%v.png",
			Fields: []string{
				"Timestamp",
				"ID",
			},
		},
	}
}

// Finalize releases resources held by app, flushes all buffers,
// closes all files and/or connections.
func (app *DemoApp) Finalize() error {
	app.Cs.Delete()
	app.Y.Delete()
	app.T.Delete()
	app.P.Delete()
	return app.O.Close()
}

// Init initializes app by applying the configuration.
func (app *DemoApp) Init() error {
	if err := app.Cs.Init(); err != nil {
		return err
	}
	if len(app.TstImg) != 0 {
		if err := app.Cs.Apply(func(c *camera.Camera) error {
			return c.TstSetImage(app.TstImg)
		}); err != nil {
			return err
		}
		log.Println("set test image: ", app.TstImg)
	}
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
	if err := app.F.Init(models.Image{}); err != nil {
		return err
	}
	return nil
}

// Run runs the obj. detection/OCR pipeline for a single image.
func (app *DemoApp) Run(res *models.Result) error {
	sw := stopwatch.New()

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
			app.P.ResetROI()
			return err
		}
		//res.Timings.Set("preprocess", sw.Lap())
		if err := app.T.Inference(app.P.GetRawImage(), tRes); err != nil {
			app.P.ResetROI()
			return err
		}
		// adjust results
		res.Text[i] = strings.Join(tRes.Text, " ")
		for _, c := range tRes.Confidences {
			res.Confidences[i] *= c / 100.0
		}
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
	res.Timings.Set("output", sw.Lap())

	//res.Timings.Set("total", sw.Total())	// tracked by the caller
	return nil
}

func runDemo(cmd *cobra.Command, args []string) error {
	// FIXME: this should be done on a per-goroutine basis, not here
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	stats := NewStats()
	sw := stopwatch.New()

	// read config
	log.Println("setting up")
	cfg, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}
	// setup the app
	app := NewDemoApp()
	defer func() {
		if err := app.Finalize(); err != nil {
			log.Println("finalization error: ", err)
		}
	}()
	// unmarshall
	if err := json.Unmarshal(cfg, &app); err != nil {
		return err
	}
	if err := app.Init(); err != nil {
		return err
	}

	log.Println("starting acquisition")
	if err := app.Cs.StartAcquisition(); err != nil {
		return err
	}
	defer app.Cs.StopAcquisition() // technically happens automatically

	log.Println("initialized")
	stats.InitDuration = sw.Lap()

	count := int64(0) // No. of successfully acquired and processed results to average
	for app.Cs.IsAcquiring() {
		stats.Result.Reset()
		stats.Result.TimeStamp = time.Now()
		sw.Lap() // reset the lap for the new acquisition loop

		// use the trigger if it's defined
		if err := app.Cs.TryTrigger(); err != nil {
			// XXX: IsAcquiring should pick up more serious errors, so
			// we should be able to ignore trigger errors entirely, but
			// not entirely sure.
			log.Println("triggering error or timed out")
			continue
		}
		log.Println("acquiring image")
		if err := app.Cs.Acquire(); err != nil {
			return err
		}
		stats.Result.Timings.Set("acquisition", sw.Lap())

		// FIXME: output/processing should not block acquisition
		for _, cam := range app.Cs {
			if cam.Result.Buffer == nil {
				continue
			}
			sw.Lap() // reset the lap to time processing

			// FIXME: the image processor shouldn't own the image
			if err := app.P.ReceiveRawImage(cam.Result); err != nil {
				return err
			}
			log.Printf("processing image: %d", cam.Result.ID)
			if err := app.Run(stats.Result); err != nil {
				log.Printf("processing error: %v", err)
				continue
			}
			stats.Result.Timings.Set("process", sw.Lap())

			log.Printf("writing image: %d", cam.Result.ID)
			var fname string
			if fname, err = app.F.Get(&cam.Result); err != nil {
				log.Printf("could not generate image filename: %v", err.Error())
				continue
			}
			if err := app.P.WriteImage(fname); err != nil {
				log.Printf("failed to write image: %v", err.Error())
			}
			stats.Result.Timings.Set("write", sw.Lap())

			count++
			stats.Accumulate(stats.Result.Timings)
		}
	}
	stats.Average(count)
	stats.ExecDuration = sw.Total()

	fmt.Println(stats)
	return nil
}
