// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package cmd

import (
	"encoding/json"
	"log"
	"math"
	"os"
	"runtime"

	"github.com/Milover/beholder/internal/camera"
	"github.com/Milover/beholder/internal/imgproc"
	"github.com/Milover/beholder/internal/models"
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
	Cs camera.Array           `json:"cameras"`
	IP *imgproc.Processor     `json:"image_processing"`
	F  Filename[models.Image] `json:"filename"`
}

// NewCamApp creates a new camera app.
func NewCamApp() *CamApp {
	return &CamApp{
		IP: imgproc.NewProcessor(),
		F: Filename[models.Image]{
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
	app.Cs.Delete()
	app.IP.Delete()
}

// Init initializes the camera app by applying the configuration.
func (app *CamApp) Init() error {
	if err := app.Cs.Init(); err != nil {
		return err
	}
	if err := app.IP.Init(); err != nil {
		return err
	}
	if err := app.F.Init(models.Image{}); err != nil {
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
	if err := app.Cs.StartAcquisition(); err != nil {
		return err
	}
	defer app.Cs.StopAcquisition() // technically happens automatically

	var acquired uint64
	for app.Cs.IsAcquiring() && acquired < nReqAcquired {
		// use the trigger if it's defined
		if err := app.Cs.TryTrigger(); err != nil {
			return err // FIXME: should probably handle timeouts gracefully
		}
		log.Println("acquiring...")
		if err := app.Cs.Acquire(); err != nil {
			return err
		}
		// FIXME: output/processing should not block acquisition
		for _, cam := range app.Cs {
			if cam.Result.Buffer == nil {
				continue
			}
			acquired++

			// FIXME: the image processor shouldn't own the image
			if err := app.IP.ReceiveRawImage(cam.Result); err != nil {
				return err
			}
			log.Println("writing image with ID: ", cam.Result.ID)
			var fname string
			if fname, err = app.F.Get(&cam.Result); err != nil {
				log.Println("could not generate image filename: ", err.Error())
				continue
			}
			if err := app.IP.WriteImage(fname); err != nil {
				log.Println("failed to write image: ", err.Error())
			}
		}
	}
	return nil
}
