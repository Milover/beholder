package cmd

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"runtime"
	"strconv"
	"time"

	"github.com/Milover/beholder/internal/camera"
	"github.com/Milover/beholder/internal/ocr"
	"github.com/spf13/cobra"
)

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

func cam(cmd *cobra.Command, args []string) error {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	log.Println("setting up")
	cfg, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}
	// setup pylon
	p := camera.NewPylon()
	defer p.Finalize()
	if err := json.Unmarshal(cfg, &p); err != nil {
		return err
	}
	if err := p.Init(); err != nil {
		return err
	}
	// setup image processor
	ip := ocr.NewImageProcessor()
	defer ip.Delete()
	// FIXME: this currently does nothing
	if err := json.Unmarshal(cfg, &ip); err != nil {
		return err
	}
	if err := ip.Init(); err != nil {
		return err
	}

	log.Println("starting acquisition")
	if err := p.C.StartAcquisition(); err != nil {
		return err
	}
	defer p.C.StopAcquisition() // technically happens automatically

	// FIXME: should be set based on configuration
	triggerTimeout, _ := time.ParseDuration("0.25s")
	for p.C.IsAcquiring() {
		log.Println("triggering...")
		// FIXME: should trigger based on configuration
		if err := p.C.WaitAndTrigger(triggerTimeout); err != nil {
			// FIXME: should probably handle timeouts gracefully
			return err
		}

		log.Println("acquiring...")
		img, err := p.C.Acquire()
		if err != nil {
			return err
		}
		if img == nil {
			continue
		}

		log.Println("writing image with ID: ", img.ID)
		// FIXME: output/processing should not block acquisition
		if img.Monochrome {
			err = ip.ReceiveMono8(img.Buffer(), img.Rows, img.Cols, img.Step)
		} else {
			err = ip.CopyBayerRGGB8(img.Buffer(), img.Rows, img.Cols, img.Step)
		}
		// FIXME: should be runtime settable
		filename := fmt.Sprintf("img_%v_%v.jpg",
			img.Timestamp.Format("2006-01-02_15-04-05"),
			strconv.FormatUint(img.ID, 10))
		if err := ip.WriteImage(filename); err != nil {
			log.Println("failed to write image")
		}
		img.Delete()
	}
	return nil
}
