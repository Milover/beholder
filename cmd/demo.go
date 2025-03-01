// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package cmd

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"math"
	"net"
	"net/http"
	"os"
	"os/signal"
	"path"
	"runtime"
	"strings"
	"syscall"
	"time"

	"github.com/Milover/beholder/internal/camera"
	"github.com/Milover/beholder/internal/imgproc"
	"github.com/Milover/beholder/internal/models"
	"github.com/Milover/beholder/internal/neural"
	"github.com/Milover/beholder/internal/output"
	"github.com/Milover/beholder/internal/server"
	"github.com/Milover/beholder/internal/stopwatch"
	"github.com/google/uuid"
	"github.com/spf13/cobra"
)

func init() {
	// Enable internal randomness pool to improve UUID generation throughput.
	//
	// FIXME: maybe think of a better place to put this, we're bound to move
	// some files around and forget that this is supposed to be turned on.
	uuid.EnableRandPool()
}

var (
	demoCmd = &cobra.Command{
		Use:   "demo [CONFIG]",
		Short: "Run obj. detection/OCR using CONFIG to configure the program",
		Long:  `Run obj. detection/OCR using CONFIG to configure the program`,
		Args: cobra.MatchAll(
			cobra.ExactArgs(1),
		),
		RunE: demoMain,
	}
)

// DemoApp is a program for showcasing image acquisition, processing and
// web serving.
//
// TODO: replace specific types with generic pipeline components.
type DemoApp struct {
	Cs camera.Array           `json:"cameras"`
	Y  *neural.YOLOv8         `json:"yolov8"`
	CR *neural.CRAFT          `json:"craft"`
	PS *neural.PARSeq         `json:"parseq"`
	P  *imgproc.Processor     `json:"image_processing"`
	O  *output.Output         `json:"output"`
	F  Filename[models.Image] `json:"filename"`

	TstImg string `json:"tst_camera_test_image"`

	// blobs are the acquired processed and encoded images, ready to be
	// distributed elsewhere.
	blobs chan *server.Blob
	// errs is the channel on which irrecoverable acquisition errors are
	// sent.
	errs chan error
	// stats is a collection of app statistics and processing results.
	stats *Stats
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
		stats: NewStats(),
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

// processImage runs the processing pipeline for a single result (image).
func (app *DemoApp) processImage(res *models.Result) error {
	sw := stopwatch.New()

	// force 3-channel image
	app.P.ToColor()

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

// acquireImages ...
// TODO: write docs
func (app *DemoApp) acquireImages() {
	defer func() {
		close(app.blobs)
		close(app.errs)
	}()
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	sw := stopwatch.New()

	log.Println("starting acquisition")
	if err := app.Cs.StartAcquisition(); err != nil {
		app.errs <- err
		return
	}
	// TODO: would be nice to communicate that acquisition has stopped
	// to other parts of the software.
	defer app.Cs.StopAcquisition()

	// BUG: stopping acquisiting with [DemoApp.StopAcquisition] gives:
	//
	// 	2024/09/26 18:05:57 triggering error or timed out
	// 	2024/09/26 18:05:57 acquiring image
	// 	could not acquire image: acquisition not started
	// 	2024/09/26 18:05:57 error: camera.Camera.Acquire: image acquisition error
	//
	// Acquisition should stop cleanly if it's stopped manually.
	for app.Cs.IsAcquiring() {
		app.stats.Result.Reset()
		app.stats.Result.Timestamp = time.Now()
		sw.Lap() // reset the lap for the new acquisition loop

		// use the trigger if it's defined
		// TODO: the trigger should enable single/multiple image acquisition
		// mode, it currently only supports continuous (infinite) acquisition.
		if err := app.Cs.TryTrigger(); err != nil {
			// TODO: would be nice to know which camera failed to trigger.
			log.Println("triggering error or timed out")
			// FIXME: IsAcquiring should pick up more serious errors, so
			// we should be able to ignore trigger errors entirely,
			// however if we fail to trigger there is no sense in trying
			// to acquire.
			// The issue is that we currently expect all cameras to trigger
			// once TryTrigger is called, so if one camera fails to trigger
			// we won't acquire any images.
			// What we should probably do is trigger the first available
			// camera, and then acquire the first available image, that is
			// we have to change how [camera.Array] functions which implies
			// modifying the C-API.
			// When this is done, the loop below won't be necessary.
			continue
		}
		log.Println("acquiring image")
		if err := app.Cs.Acquire(); err != nil {
			// TODO: would be nice to know which camera failed to acquire.
			app.errs <- err
			return
		}
		app.stats.Result.Timings.Set("acquisition", sw.Lap())

		// FIXME: output/processing should not block acquisition
		for _, cam := range app.Cs {
			if cam.Result.Buffer == nil {
				continue
			}
			sw.Lap() // reset the lap to time processing

			// FIXME: the image processor shouldn't own the image
			if err := app.P.ReceiveRawImage(cam.Result); err != nil {
				// TODO: would be nice to know which camera failed to
				// yield an image.
				app.errs <- err
				return
			}
			log.Printf("processing image: %d", cam.Result.ID)
			if err := app.processImage(app.stats.Result); err != nil {
				// TODO: would be nice to know from which camera the failed
				// image came from.
				log.Printf("processing error: %v", err)
				continue
			}
			app.stats.Result.Timings.Set("process", sw.Lap())

			// FIXME: encoding/writing should not block acquisition/processing.
			var err error
			var fname string
			if fname, err = app.F.Get(&cam.Result); err != nil {
				log.Printf("could not generate image filename: %v", err)
			}
			app.stats.Result.Timings.Set("gen-fname", sw.Lap())

			var encoding []byte
			if encoding, err = app.P.EncodeImage(path.Ext(fname)); err != nil {
				log.Printf("failed to encode image: %v", err)
				app.stats.RollingAverage(app.stats.Result.Timings)
				continue
			}
			app.stats.Result.Timings.Set("encode", sw.Lap())

			if err := os.WriteFile(fname, encoding, 0644); err != nil {
				log.Printf("failed to write image: %v", err)
			}
			app.stats.Result.Timings.Set("write", sw.Lap())

			blob := &server.Blob{
				UUID:   uuid.Must(uuid.NewV7()), // FIXME: should take img.UUID
				Source: cam.SN,
				Bytes:  encoding,
			}
			app.blobs <- blob
			app.stats.Result.Timings.Set("ch-send", sw.Lap())

			app.stats.RollingAverage(app.stats.Result.Timings)
		}
	}
}

// IsAcquiring reports whether image acquisition is currently running.
func (app *DemoApp) IsAcquiring() bool {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()
	return app.Cs.IsAcquiring()
}

// StartAcquisition starts the image acquisition and sets up the channels
// on which acquired images and errors are sent.
func (app *DemoApp) StartAcquisition(blobsSize int) (<-chan *server.Blob, <-chan error) {
	if app.IsAcquiring() {
		return app.blobs, app.errs
	}
	app.blobs = make(chan *server.Blob, blobsSize)
	app.errs = make(chan error, 1)
	go app.acquireImages()

	return app.blobs, app.errs
}

// StopAcquisition stops the image acquisition process.
//
// NOTE: currently we rely on [DemoApp.acquireImages] to detect acquisition
// stoppage and close any channels currently in use, which might not be
// the best idea.
func (app *DemoApp) StopAcquisition() {
	app.Cs.StopAcquisition()
}

// demoMain reads the runtime configuration and sets up and runs the program.
func demoMain(cmd *cobra.Command, args []string) error {
	sw := stopwatch.New()

	// read config
	log.Println("setting up")
	cfg, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}

	// setup app
	// TODO: app setup should probably go in a separate go routine, so that
	// OS-thread-locking/unlocking doesn't have to happen here.
	runtime.LockOSThread()
	app := NewDemoApp()
	defer func() {
		// deferred C-call so we don't care if it's locked or not
		if err := app.Finalize(); err != nil {
			log.Printf("app finalization error: %v", err)
		}
	}()
	if err := json.Unmarshal(cfg, &app); err != nil {
		return err
	}
	if err := app.Init(); err != nil {
		return err
	}
	runtime.UnlockOSThread()
	// average and dump stats when we're done
	defer func() {
		app.stats.ExecDuration = sw.Total()
		fmt.Println(app.stats)
	}()

	// set up server
	//
	// TODO: whether a server gets set up should be runtime configurable since:
	//	1. some applications might not need an HTTP server at all
	//	2. acquisition server API (web UI) should be exposed only
	//     in controlled (development) environments.
	ln, err := net.Listen("tcp", ":8080") // TODO: should be configurable
	if err != nil {
		return err
	}
	log.Printf("listening on %v", ln.Addr())

	as, err := server.NewAcquisitionServer(app)
	if err != nil {
		return err
	}
	as.Start()
	defer func() {
		if err := as.Close(); err != nil {
			log.Printf("acquisition server close error: %v", err)
		}
	}()
	// TODO: should probably handle static content (file server) routes here.
	srv := &http.Server{
		Handler:      as,
		ReadTimeout:  time.Second * 10, // TODO: should be configurable
		WriteTimeout: time.Second * 10, // TODO: should be configurable
	}
	// serve HTTP
	go func() {
		if err := srv.Serve(ln); !errors.Is(err, http.ErrServerClosed) {
			// most examples Fatalf here, but that doesn't seem right
			log.Printf("serve error: %v", err)
		}
	}()

	// set up signal handling
	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	log.Println("initialized")
	app.stats.InitDuration = sw.Lap()

	// handle signals and clean up
	<-ctx.Done()

	shutdownCtx, shutdownCancel := context.WithTimeout(ctx, 5*time.Second)
	defer shutdownCancel()
	if err := srv.Shutdown(shutdownCtx); err != nil {
		log.Printf("server shutdown error: %v", err)
	}
	return nil
}
