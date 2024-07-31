package cmd

import (
	"encoding/json"
	"log"
	"math"
	"os"
	"runtime"

	"github.com/Milover/beholder/internal/camera"
	"github.com/Milover/beholder/internal/ocr"
	"github.com/spf13/cobra"
)

var (
	// nImages is the needed number of successfully acquired images for the
	// program to complete.
	// It is set to 'infinity' by default, i.e. the program will continually
	// acquire images withouth stopping.
	nReqAcquired uint64
)

func init() {
	camCmd.Flags().Uint64Var(
		&nReqAcquired,
		"n-img",
		math.MaxUint64,
		"number of images to successfully acquire before exiting",
	)
}

var (
	camCmd = &cobra.Command{
		Use:   "cam [CONFIG]",
		Short: "Acquire images from a camera configured with CONFIG",
		Long:  "Acquire images from a camera configured with CONFIG",
		Args: cobra.MatchAll(
			cobra.ExactArgs(1),
		),
		RunE: cam,
	}
)

// CamApp represents a program for acquiring and outputing images from
// a camera device.
type CamApp struct {
	TL *camera.TransportLayer  `json:"transport_layer"`
	C  *camera.Camera          `json:"camera"`
	IP *ocr.ImageProcessor     `json:"image_processing"`
	F  Filename[camera.Result] `json:"filename"`
}

// NewCamApp creates a new camera app.
func NewCamApp() *CamApp {
	return &CamApp{
		TL: camera.NewTransportLayer(),
		C:  camera.NewCamera(),
		IP: ocr.NewImageProcessor(),
		F: Filename[camera.Result]{
			FString: "img_%v_%v.png",
			Fields: []string{
				"Timestamp",
				"ID",
			},
		},
	}
}

// Finalize releases resources held by the camera app.
func (app *CamApp) Finalize() {
	app.TL.Delete()
	app.C.Delete()
	app.IP.Delete()
}

// Init initializes the camera app by applying the configuration.
func (app *CamApp) Init() error {
	if err := app.TL.Init(); err != nil {
		return err
	}
	if err := app.C.Init(*app.TL); err != nil {
		return err
	}
	if err := app.IP.Init(); err != nil {
		return err
	}
	if err := app.F.Init(camera.Result{}); err != nil {
		return err
	}
	return nil
}

func cam(cmd *cobra.Command, args []string) error {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	log.Println("setting up")
	cfg, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}
	app := NewCamApp()
	defer app.Finalize()
	if err := json.Unmarshal(cfg, &app); err != nil {
		return err
	}
	if err := app.Init(); err != nil {
		return err
	}

	log.Println("starting acquisition")
	if err := app.C.StartAcquisition(); err != nil {
		return err
	}
	defer app.C.StopAcquisition() // technically happens automatically

	var acquired uint64
	for app.C.IsAcquiring() && acquired < nReqAcquired {
		// use the trigger if it's defined
		if err := app.C.TryTrigger(); err != nil {
			return err // FIXME: should probably handle timeouts gracefully
		}
		log.Println("acquiring...")
		err := app.C.Acquire()
		if err != nil {
			return err
		}
		if app.C.Result.Value == nil {
			continue
		}
		acquired++

		if err := app.IP.ReceiveAcquisitionResult(app.C.Result.Value); err != nil {
			return err
		}
		// FIXME: output/processing should not block acquisition
		log.Println("writing image with ID: ", app.C.Result.ID)
		var fname string
		if fname, err = app.F.Get(&app.C.Result); err != nil {
			log.Println("could not generate image filename: ", err.Error())
			continue
		}
		if err := app.IP.WriteImage(fname); err != nil {
			log.Println("failed to write image: ", err.Error())
		}
	}
	return nil
}
