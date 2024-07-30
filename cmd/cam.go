package cmd

import (
	"encoding/json"
	"fmt"
	"log"
	"math"
	"os"
	"path"
	"reflect"
	"runtime"
	"strconv"
	"time"

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

// TODO: move this to an appropriate package.
// Filename is the runtime configuration for generating file names when
// writing images to disc.
type Filename struct {
	// FString is the format string used to generate the image file name.
	// It can be either a relative or an absolute path, as long as it
	// is valid, ie. the directory exists.
	// NOTE: the file extension determines the image format.
	FString string `json:"f-string"`
	// Fields are the names of camera.Image fields used as input to FString.
	Fields []string `json:"fields"`
}

// NewFilename creates a new default Filename.
func NewFilename() Filename {
	return Filename{
		FString: "img_%v_%v.jpg",
		Fields: []string{
			"Timestamp",
			"ID",
		},
	}
}

// FIXME: reimplement so that 'reflect' is used only at setup
func (f Filename) Values(img camera.Image) ([]any, error) {
	val := reflect.ValueOf(img)
	fieldVals := make([]any, 0, len(f.Fields))
	for _, field := range f.Fields {
		fld := val.FieldByName(field)
		if !fld.IsValid() {
			return nil, fmt.Errorf("invalid Filename field: %q", field)
		}
		switch fld.Kind() {
		case reflect.Uint64:
			fieldVals = append(fieldVals, strconv.FormatUint(fld.Uint(), 10))
		case reflect.Int64:
			fieldVals = append(fieldVals, strconv.FormatInt(fld.Int(), 10))
		case reflect.Bool:
			fieldVals = append(fieldVals, strconv.FormatBool(fld.Bool()))
		case reflect.Struct:
			if fld.Type() == reflect.TypeOf(time.Time{}) {
				fieldVals = append(fieldVals, fld.Interface().(time.Time).Format("2006-01-02_15-04-05"))
			} else {
				return nil, fmt.Errorf("Filename field %q if of unsupported struct type: %q", field, fld.Type())
			}
		default:
			return nil, fmt.Errorf("Filename field %q is of unsupported type: %q", field, fld.Kind())
		}
	}
	return fieldVals, nil
}

// IsValid is an assertion that f is designates a valid file path, and
// that f.Fields exist as fields of camera.Image.
func (f Filename) Get(img camera.Image) (string, error) {
	fieldVals, err := f.Values(img)
	if err != nil {
		return "", err
	}
	filename := fmt.Sprintf(f.FString, fieldVals...)
	if _, err := os.Stat(path.Dir(filename)); err != nil {
		return "", err
	}
	return filename, nil
}

type CamCmdStruct struct {
	P  camera.Pylon `json:"pylon"`
	F  Filename     `json:"filename"`
	IP *ocr.ImageProcessor
}

func cam(cmd *cobra.Command, args []string) error {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	log.Println("setting up")
	cfg, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}
	ccs := CamCmdStruct{
		P:  camera.NewPylon(),
		F:  NewFilename(),
		IP: ocr.NewImageProcessor(),
	}
	defer ccs.P.Finalize()
	defer ccs.IP.Delete()
	if err := json.Unmarshal(cfg, &ccs); err != nil {
		return err
	}
	if err := ccs.P.Init(); err != nil {
		return err
	}
	if err := ccs.IP.Init(); err != nil {
		return err
	}

	log.Println("starting acquisition")
	if err := ccs.P.C.StartAcquisition(); err != nil {
		return err
	}
	defer ccs.P.C.StopAcquisition() // technically happens automatically

	var acquired uint64
	for ccs.P.C.IsAcquiring() && acquired < nReqAcquired {
		// use the trigger if it's defined
		if err := ccs.P.C.TryTrigger(); err != nil {
			return err // FIXME: should probably handle timeouts gracefully
		}
		log.Println("acquiring...")
		img, err := ccs.P.C.Acquire()
		if err != nil {
			return err
		}
		if img == nil {
			continue
		}
		acquired++

		// FIXME: output/processing should not block acquisition
		log.Println("writing image with ID: ", img.ID)
		if img.Monochrome {
			err = ccs.IP.ReceiveMono8(img.Buffer(), img.Rows, img.Cols, img.Step)
		} else {
			err = ccs.IP.CopyBayerRGGB8(img.Buffer(), img.Rows, img.Cols, img.Step)
		}
		if fname, err := ccs.F.Get(*img); err != nil {
		} else {
			if err := ccs.IP.WriteImage(fname); err != nil {
				log.Println("failed to write image")
			}
		}
		img.Delete()
	}
	return nil
}
