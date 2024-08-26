package cmd

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"os"
	"path"
	"runtime"

	"github.com/Milover/beholder/internal/image"
	"github.com/Milover/beholder/internal/neutral"
	"github.com/Milover/beholder/internal/stopwatch"
	"github.com/spf13/cobra"
)

var (
	procCmd = &cobra.Command{
		Use:   "proc [CONFIG] [FILE/DIRECTORY]",
		Short: "Run image processing pipeline from CONFIG on FILE or all files in DIRECTORY",
		Long:  `Run image processing pipeline from CONFIG on FILE or all files in DIRECTORY`,
		Args: cobra.MatchAll(
			cobra.ExactArgs(2),
		),
		RunE: runProc,
	}
)

// id is a helper type, so that Filename can receive an arbitrary string.
type id struct {
	ID string // an arbitrary string ID
}

// ProcApp represents a program for running an image processing pipeline
// on an image or a set of images read from disc.
type ProcApp struct {
	P *image.Processor `json:"image_processing"`
	F Filename[id]     `json:"filename"`
}

// NewProcApp creates a new Proc app.
func NewProcApp() *ProcApp {
	return &ProcApp{
		P: image.NewProcessor(),
		F: Filename[id]{
			FString: "img_%v.jpeg",
			Fields:  []string{"ID"},
		},
	}
}

// Finalize releases resources held by the Proc app, flushes all buffer,
// closes all files and/or connections.
func (app *ProcApp) Finalize() error {
	app.P.Delete()
	return nil
}

// Init initializes the Proc app by applying the configuration.
func (app *ProcApp) Init() error {
	if err := app.P.Init(); err != nil {
		return err
	}
	if err := app.F.Init(id{}); err != nil {
		return err
	}
	return nil
}

// Run is a function that runs the image processing pipeline for
// a single image file: reading and preprocessing.
func (app *ProcApp) Run(filename string, imgID id, res *neutral.Result) error {
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
	if err := app.P.DecodeImage(buf, image.RMAnyColor); err != nil {
		return err
	}
	res.Timings.Set("decode", sw.Lap())

	if err := app.P.Preprocess(); err != nil {
		return err
	}
	res.Timings.Set("preprocess", sw.Lap())

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

func runProc(cmd *cobra.Command, args []string) error {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	stats := NewStats()
	sw := stopwatch.New()

	// read config
	cfg, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}
	// setup Proc
	app := NewProcApp()
	defer func() {
		if err := app.Finalize(); err != nil {
			log.Println("Proc app finalization error:", err)
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
		id := id{ID: fmt.Sprintf("%07d", count)}
		if err = app.Run(cliFile, id, stats.Result); err != nil {
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
			id := id{ID: fmt.Sprintf("%07d", count)}
			file := path.Join(dir.Name(), filename)

			log.Printf("processing (%d/%d): %v", count+1, len(filenames), file)
			if err := app.Run(file, id, stats.Result); err != nil {
				//return err
				log.Println("processing error:", err)
				continue
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
