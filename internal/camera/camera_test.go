package camera

import (
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"path"
	"strconv"
	"testing"

	"github.com/Milover/beholder/internal/ocr"
	"github.com/stretchr/testify/assert"
)

// Test MAC address validation.
type macAddrTest struct {
	Name     string
	Input    string
	Expected bool
}

var macAddrTests = []macAddrTest{
	{
		Name:     "good-colons",
		Input:    "01:23:45:67:89:AB",
		Expected: true,
	},
	{
		Name:     "good-colons-lowercase",
		Input:    "01:f3:ab:67:89:ab",
		Expected: true,
	},
	{
		Name:     "good-hyphens",
		Input:    "01-23-45-67-89-AB",
		Expected: true,
	},
	{
		Name:     "good-no-separator",
		Input:    "0123456789AB",
		Expected: true,
	},
	{
		Name:     "bad-separator",
		Input:    "0123.4567.89AB",
		Expected: false,
	},
	{
		Name:     "bad-byte-value-colons",
		Input:    "01:23:45:67:89:ZZ",
		Expected: false,
	},
	{
		Name:     "bad-byte-value-no-separators",
		Input:    "0123456789XYZ",
		Expected: false,
	},
}

func TestIsValidMAC(t *testing.T) {
	for _, tt := range macAddrTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)
			assert.Equal(tt.Expected, IsValidMAC(tt.Input))
		})
	}
}

// Set up cmd-line flags for the camera tests
var (
	// macAddr is the MAC address of the camera which is used for testing.
	// If it is not supplied as a command line flag, the one defined in
	// the test configuration is used.
	macAddr string
	// hwTriggering determines whether to run tests which use
	// hardware triggering for image acquisition.
	// Hardware triggering tests are skipped by default.
	hwTriggering bool
	// nReqImgs determines how many images need to be successfully acquired
	// before a test completes.
	// Tests complete after 3 images are acquired succesfully by default.
	nReqImgs uint64
	// cleanUp determines whether to delete images written during a test
	// after the test completes.
	// Written images are deleted after a test completes by default.
	cleanUp bool
)

func init() {
	flag.StringVar(&macAddr, "mac", "", "MAC address of camera used in tests")
	flag.BoolVar(&hwTriggering, "hw-trigger", false, "run tests which use hardware triggering")
	flag.Uint64Var(&nReqImgs, "n-img", 3, "number of successful image acquisitions needed to complete a test")
	flag.BoolVar(&cleanUp, "cleanup", true, "delete images after a test completes")
}

func TestMain(m *testing.M) {
	flag.Parse()
	// fail right away if the cmdline supplied MAC is bad
	if len(macAddr) != 0 && !IsValidMAC(macAddr) {
		log.Fatalf("bad command line MAC address: %q", macAddr)
	}
	code := m.Run()
	os.Exit(code)
}

// Test image acquisition
type cameraTest struct {
	Name           string
	Error          error
	Config         string
	NeedsHwTrigger bool
}

var cameraTests = []cameraTest{
	{
		Name:  "basic-software-trigger",
		Error: nil,
		Config: `
{
	"camera": {
		"mac": "00:30:53:44:87:E9",
		"acquisition_timeout": "2s",
		"trigger": {
			"timeout": "2s",
			"period": "1s"
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
		Name:           "basic-hardware-trigger",
		Error:          nil,
		NeedsHwTrigger: true,
		Config: `
{
	"camera": {
		"mac": "00:30:53:44:87:E9",
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

func TestPylon(t *testing.T) {
	for _, tt := range cameraTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			// configure the test
			if tt.NeedsHwTrigger && hwTriggering == false {
				t.Skip("hardware triggering is disabled")
			}
			var outDir string
			if cleanUp {
				outDir = t.TempDir()
			}

			// setup
			p := struct {
				TL *TransportLayer     `json:"transport_layer"`
				C  *Camera             `json:"camera"`
				IP *ocr.ImageProcessor `json:"image_processor"`
			}{
				TL: NewTransportLayer(),
				C:  NewCamera(),
				IP: ocr.NewImageProcessor(),
			}
			defer func() {
				p.C.Delete()
				p.TL.Delete()
				p.IP.Delete()
			}()
			// unmarshal
			err := json.Unmarshal([]byte(tt.Config), &p)
			assert.Nil(err, err)
			// switch MAC address if necessary
			if len(macAddr) != 0 {
				p.C.MAC = macAddr
			}
			// initialize
			err = func() error {
				if err := p.TL.Init(); err != nil {
					return err
				}
				if err := p.C.Init(*p.TL); err != nil {
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
					log.Println("waiting for trigger...")
					err = p.C.TryTrigger()
					assert.Nil(err, err)
					log.Println("trigger fired")
				}
				log.Println("acquiring...")
				err := p.C.Acquire()
				assert.Nil(err, err)
				if p.C.Result.Value == nil {
					continue
				}
				nAcquired++

				log.Println("writing...")
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
