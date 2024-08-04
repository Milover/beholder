package camera

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"path"
	"strconv"
	"testing"

	"github.com/Milover/beholder/internal/ocr"
	"github.com/stretchr/testify/assert"
)

// A hack to enable camera emulation during testing.
//
// This variable is evaluated before any init() functions are run, and thus
// ensures that PYLON_CAMEMU is present in the environment before
// the pylon API is initialized.
var _ = func() (_ struct{}) {
	if err := os.Setenv("PYLON_CAMEMU", "3"); err != nil {
		panic("could not set 'PYLON_CAMEMU'")
	}
	return
}()

// Command line flags for the camera tests.
var (
	// serialNo is the serial number of the camera which is used for testing.
	// If it is not supplied as a command line flag, the one defined in
	// the test configuration is used.
	serialNo string
	// emuOnly determines whether to only run tests which use an emulated
	// camera device.
	// Only tests using an emulated camera device are run by default.
	emuOnly bool
	// hwTriggering determines whether to run tests which use
	// hardware triggering for image acquisition.
	// Hardware triggering tests are skipped by default.
	hwTriggering bool
	// nReqImgs determines how many images need to be successfully acquired
	// before a test completes.
	// Tests complete after 3 images are succesfully acquired by default.
	nReqImgs uint64
	// cleanUp determines whether to delete images written during a test
	// after the test completes.
	// Written images are deleted after a test completes by default.
	cleanUp bool
)

func init() {
	flag.StringVar(&serialNo, "sn", "", "serial number of camera used in tests")
	flag.BoolVar(&emuOnly, "emu-only", true, "only run tests using an emulated camera device")
	flag.BoolVar(&hwTriggering, "hw-trigger", false, "run tests which use hardware triggering")
	flag.Uint64Var(&nReqImgs, "n-img", 3, "number of successful image acquisitions needed to complete a test")
	flag.BoolVar(&cleanUp, "cleanup", true, "delete images after a test completes")
}

// TestMain parses command line flags for the tests.
func TestMain(m *testing.M) {
	flag.Parse()
	code := m.Run()
	os.Exit(code)
}

// Test image acquisition pipeline.
type cameraTest struct {
	Name           string
	Error          error
	Config         string
	NeedsHwTrigger bool // does the test need a hardware trigger?
	NonEmulated    bool // does the test use a physical camera device?
}

var cameraTests = []cameraTest{
	{
		Name:  "pick-first-emulated-w/-software-trigger",
		Error: nil,
		Config: `
{
	"camera": {
		"type": "emulated",
		"serial_number": "pick-first",
		"acquisition_timeout": "1s",
		"trigger": {
			"timeout": "1s",
			"period": "0.5s"
		},
		"parameters": [
			{"name": "AcquisitionMode",    "value": "Continuous"},
			{"name": "TriggerSelector",    "value": "FrameStart"},
			{"name": "TriggerMode",        "value": "On"},
			{"name": "TriggerSource",      "value": "Software"},
			{"name": "ExposureMode",       "value": "Timed"},
			{"name": "ExposureTimeAbs",    "value": "10000"}
		]
	}
}
`,
	},
	{
		Name:  "single-emulated-w/-software-trigger",
		Error: nil,
		Config: `
{
	"camera": {
		"type": "emulated",
		"serial_number": "0815-0000",
		"acquisition_timeout": "1s",
		"trigger": {
			"timeout": "1s",
			"period": "0.5s"
		},
		"parameters": [
			{"name": "AcquisitionMode",    "value": "Continuous"},
			{"name": "TriggerSelector",    "value": "FrameStart"},
			{"name": "TriggerMode",        "value": "On"},
			{"name": "TriggerSource",      "value": "Software"},
			{"name": "ExposureMode",       "value": "Timed"},
			{"name": "ExposureTimeAbs",    "value": "10000"}
		]
	}
}
`,
	},
	{
		Name:        "pick-first-gige-w/-software-trigger",
		Error:       nil,
		NonEmulated: true,
		Config: `
{
	"camera": {
		"type": "gige",
		"serial_number": "pick-first",
		"acquisition_timeout": "1s",
		"trigger": {
			"timeout": "1s",
			"period": "0.5s"
		},
		"parameters": [
			{"name": "AcquisitionMode",    "value": "Continuous"},
			{"name": "TriggerSelector",    "value": "FrameStart"},
			{"name": "TriggerMode",        "value": "On"},
			{"name": "TriggerSource",      "value": "Software"},
			{"name": "ExposureMode",       "value": "Timed"},
			{"name": "ExposureTimeAbs",    "value": "10000"},
			{"name": "ChunkModeActive",    "value": "true"},
			{"name": "ChunkSelector",      "value": "PayloadCRC16"},
			{"name": "ChunkEnable",        "value": "true"}
		]
	}
}
`,
	},
	{
		Name:           "pick-first-gige-w/-hardware-trigger",
		Error:          nil,
		NonEmulated:    true,
		NeedsHwTrigger: true,
		Config: `
{
	"camera": {
		"type": "gige",
		"serial_number": "pick-first",
		"parameters": [
			{"name": "AcquisitionMode",    "value": "Continuous"},
			{"name": "TriggerSelector",    "value": "FrameStart"},
			{"name": "TriggerMode",        "value": "On"},
			{"name": "TriggerSource",      "value": "Line1"},
			{"name": "TriggerActivation",  "value": "RisingEdge"},
			{"name": "LineSelector",       "value": "Line2"},
			{"name": "LineMode",           "value": "Output"},
			{"name": "LineSource",         "value": "FrameTriggerWait"},
			{"name": "LineInverter",       "value": "true"},
			{"name": "ExposureMode",       "value": "Timed"},
			{"name": "ExposureTimeAbs",    "value": "10000"},
			{"name": "ChunkModeActive",    "value": "true"},
			{"name": "ChunkSelector",      "value": "PayloadCRC16"},
			{"name": "ChunkEnable",        "value": "true"}
		]
	}
}
`,
	},
}

// TestCamera tests
func TestCamera(t *testing.T) {
	for _, tt := range cameraTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			// configure the test
			if tt.NonEmulated && emuOnly {
				t.Skip("physical camera testing is disabled")
			}
			if tt.NeedsHwTrigger && !hwTriggering {
				t.Skip("hardware triggering is disabled")
			}
			var outDir string
			if cleanUp {
				outDir = t.TempDir()
			}

			// setup
			p := struct {
				C  *Camera             `json:"camera"`
				IP *ocr.ImageProcessor `json:"image_processor"`
			}{
				C:  NewCamera(),
				IP: ocr.NewImageProcessor(),
			}
			defer func() {
				p.C.Delete()
				p.IP.Delete()
			}()
			// unmarshal
			err := json.Unmarshal([]byte(tt.Config), &p)
			assert.Nil(err, err)
			// set serial number if defined
			if len(serialNo) != 0 {
				p.C.SN = serialNo
			}
			// initialize
			err = func() error {
				if err := p.C.Init(); err != nil {
					return err
				}
				if err := p.IP.Init(); err != nil {
					return err
				}
				return nil
			}()
			assert.Nil(err, err)

			// try to acquire images
			err = p.C.StartAcquisition()
			assert.Nil(err, err)
			defer p.C.StopAcquisition() // happens automatically

			var nAcquired uint64
			for p.C.IsAcquiring() && nAcquired < nReqImgs {
				if !tt.NeedsHwTrigger {
					t.Log("waiting for trigger...")
					err = p.C.TryTrigger()
					assert.Nil(err, err)
					t.Log("trigger fired")
				}
				t.Log("acquiring...")
				err := p.C.Acquire()
				assert.Nil(err, err)
				if p.C.Result.Value == nil {
					continue
				}
				nAcquired++

				// TODO: should be in a separate test probably
				t.Log("writing...")
				filename := fmt.Sprintf("img_%v_%v.png",
					p.C.Result.Timestamp.Format("2006-01-02_15-04-05"),
					strconv.FormatUint(p.C.Result.ID, 10))
				err = p.IP.WriteAcquisitionResult(p.C.Result.Value,
					path.Join(outDir, filename))
				assert.Nil(err, err)
			}

			assert.ErrorIs(err, tt.Error, "unexpected error")
		})
	}
}
