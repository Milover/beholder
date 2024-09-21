package cmd

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"html/template"
	"log"
	"math"
	"net/http"
	"os"
	"os/signal"
	"runtime"
	"strings"
	"syscall"
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

type streamImage struct {
	Buffer []byte // raw image bytes
	MIME   string // image MIME type
}

// DemoApp is a program for showcasing image acquisition, processing and
// web serving.
type DemoApp struct {
	Cs camera.Array           `json:"cameras"`
	Y  *neural.YOLOv8         `json:"yolov8"`
	CR *neural.CRAFT          `json:"craft"`
	PS *neural.PARSeq         `json:"parseq"`
	P  *imgproc.Processor     `json:"image_processing"`
	O  *output.Output         `json:"output"`
	F  Filename[models.Image] `json:"filename"`

	TstImg string `json:"tst_camera_test_image"`

	LatestImg streamImage `json:"-"` // the latest processed and encoded image
}

// NewDemoApp creates a new demo app.
func NewDemoApp() *DemoApp {
	return &DemoApp{
		Y:  neural.NewYOLOv8(),
		CR: neural.NewCRAFT(),
		PS: neural.NewPARSeq(),
		P:  imgproc.NewProcessor(),
		O:  output.NewOutput(),
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
	app.CR.Delete()
	app.PS.Delete()
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
	if err := app.CR.Init(); err != nil {
		return err
	}
	if err := app.PS.Init(); err != nil {
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

// ProcessImage runs the obj. detection/OCR pipeline for a single image.
func (app *DemoApp) ProcessImage(res *models.Result) error {
	sw := stopwatch.New()

	// preprocess
	if err := app.P.Preprocess(); err != nil {
		return err
	}
	res.Timings.Set("preprocess", sw.Lap())

	// detect
	if err := app.Y.Inference(app.P.GetRawImage(), res); err != nil {
		log.Printf("object detection error: %v", err)
		return nil
	}
	res.Timings.Set("yolo", sw.Lap())

	// loop for each yolo ROI
	for i := range res.Boxes {
		res.Boxes[i].Resize(int64(math.Floor(
			0.05 * float64(min(res.Boxes[i].Height(),
				res.Boxes[i].Width())),
		)))
		//res.Boxes[i].Resize(15)
		app.P.SetROI(res.Boxes[i])

		eRes := models.NewResult()
		if err := app.CR.Inference(app.P.GetRawImage(), eRes); err != nil {
			log.Printf("text detection error: %v", err)
			continue
		}

		// loop for each craft ROI
		tRes := models.NewResult()
		var ts []string
		for ei, eb := range eRes.Boxes {
			eb.Move(res.Boxes[i].Left, res.Boxes[i].Top)
			eb.Resize(int64(math.Floor(0.05 * float64(min(eb.Height(), eb.Width())))))
			app.P.SetRotatedROI(eb, eRes.Angles[ei])

			if err := app.PS.Inference(app.P.GetRawImage(), tRes); err != nil {
				log.Printf("text recognition error: %v", err)
			}
			ts = append(ts, tRes.Text...)
			for _, tc := range tRes.Confidences {
				res.Confidences[i] *= tc
			}
		}
		res.Text[i] = strings.Join(ts, " ")
	}
	app.P.ResetROI()
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

// AcquireImage ...
func (app *DemoApp) AcquireImage() error {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	sw := stopwatch.New()

	log.Println("starting acquisition")
	if err := app.Cs.StartAcquisition(); err != nil {
		return err
	}
	defer app.Cs.StopAcquisition()

	// FIXME: one click - one image, no continuous acquisition
	stats.Result.Reset()
	stats.Result.TimeStamp = time.Now()
	sw.Lap() // reset the lap for the new acquisition loop

	// use the trigger if it's defined
	if err := app.Cs.TryTrigger(); err != nil {
		// XXX: IsAcquiring should pick up more serious errors, so
		// we should be able to ignore trigger errors entirely, but
		// not entirely sure.
		log.Println("triggering error or timed out")
		return nil
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
		if err := app.ProcessImage(stats.Result); err != nil {
			log.Printf("processing error: %v", err)
			continue
		}
		stats.Result.Timings.Set("process", sw.Lap())

		// FIXME: encoding should not block acquisition/processing.
		var err error
		app.LatestImg = streamImage{MIME: "image/jpeg"}
		if app.LatestImg.Buffer, err = app.P.EncodeImage(".jpeg"); err != nil {
			log.Printf("failed to encode image: %v", err)
		}
		stats.Result.Timings.Set("encode", sw.Lap())

		// FIXME: writing should not block acquisition/processing.
		var fname string
		if fname, err = app.F.Get(&cam.Result); err != nil {
			log.Printf("could not generate image filename: %v", err)
		}
		if err := os.WriteFile(fname, app.LatestImg.Buffer, 0644); err != nil {
			log.Printf("failed to write image: %v", err)
		}
		stats.Result.Timings.Set("write", sw.Lap())
		stats.RollingAverage(stats.Result.Timings)
	}
	return nil
}

var (
	// stats is a collection of app statistics and processing results.
	//
	// FIXME: bro, what the actual fuck
	stats = NewStats()

	// tmpl is the demo webpage HTML template.
	tmpl = template.Must(template.ParseFiles("web/templates/demo.html"))
)

// appRootHandler handles the "/" endpoint of the demo webpage.
func appRootHandler(app *DemoApp) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		tmpl.Execute(w, app)
	}
}

// appAcquireHandler handles the "/acquire" endpoint of the demo webpage.
func appAcquireHandler(app *DemoApp) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		done := make(chan error)
		go func() {
			done <- app.AcquireImage()
		}()
		// wait for acquisiton to finish and check for errors
		if err := <-done; err != nil {
			log.Printf("image acquisition error: %v", err)
			http.Error(w, "failed to acquire image", http.StatusInternalServerError)
			return
		}
		http.Redirect(w, r, "/", http.StatusSeeOther)
	}
}

// appStreamHandler handles the "/stream" endpoint of the demo webpage.
func appStreamHandler(app *DemoApp) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if len(app.LatestImg.Buffer) == 0 {
			http.Error(w, "cannot display image", http.StatusNotFound)
			return
		}
		w.Header().Set("Content-Type", app.LatestImg.MIME)
		w.Write(app.LatestImg.Buffer)
	}
}

func runDemo(cmd *cobra.Command, args []string) error {
	sw := stopwatch.New()
	// average and dump stats when we're done
	defer func() {
		stats.ExecDuration = sw.Total()
		fmt.Println(stats)
	}()

	// read config
	log.Println("setting up")
	cfg, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}

	// setup app
	runtime.LockOSThread() // FIXME: this should go in a separate go routine
	app := NewDemoApp()
	defer func() {
		// deferred C-call so we don't care if it's locked or not
		if err := app.Finalize(); err != nil {
			log.Println("finalization error: ", err)
		}
	}()
	if err := json.Unmarshal(cfg, &app); err != nil {
		return err
	}
	if err := app.Init(); err != nil {
		return err
	}
	defer runtime.UnlockOSThread() // FIXME: this should go in a separate goroutine

	// set up routes
	mux := http.NewServeMux()
	mux.HandleFunc("/{$}", appRootHandler(app))
	mux.HandleFunc("POST /acquire", appAcquireHandler(app))
	mux.HandleFunc("GET /stream", appStreamHandler(app))
	// set up server
	server := &http.Server{
		Addr:    ":8080",
		Handler: mux,
	}

	// set up signal handling
	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	log.Println("initialized")
	stats.InitDuration = sw.Lap()

	// serve HTTP
	go func() {
		if err := server.ListenAndServe(); !errors.Is(err, http.ErrServerClosed) {
			// most examples Fatalf here, but that doesn't seem right
			log.Printf("listen and serve error: %v", err)
		}
	}()
	log.Printf("serving on %v", server.Addr)

	// handle signals and clean up
	<-ctx.Done()

	shutdownCtx, shutdownCancel := context.WithTimeout(ctx, 5*time.Second)
	defer shutdownCancel()
	if err := server.Shutdown(shutdownCtx); err != nil {
		log.Printf("server shutdown error: %v", err)
	}

	return nil
}
