package camera

import (
	"encoding/json"
	"testing"

	"github.com/Milover/beholder/internal/ocr"
	"github.com/stretchr/testify/assert"
)

type arrayTest struct {
	Name        string
	Error       error
	Config      string
	NonEmulated bool // does the test use a physical camera device?
}

var arrayTests = []arrayTest{
	{
		Name:  "emulated-w/-software-trigger",
		Error: nil,
		Config: `
{
	"cameras": [
		{
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
				{"name": "TriggerSource",      "value": "Software"}
			]
		},
		{
			"type": "emulated",
			"serial_number": "0815-0001",
			"acquisition_timeout": "1s",
			"trigger": {
				"timeout": "1s",
				"period": "0.5s"
			},
			"parameters": [
				{"name": "AcquisitionMode",    "value": "Continuous"},
				{"name": "TriggerSelector",    "value": "FrameStart"},
				{"name": "TriggerMode",        "value": "On"},
				{"name": "TriggerSource",      "value": "Software"}
			]
		},
		{
			"type": "emulated",
			"serial_number": "0815-0001",
			"acquisition_timeout": "1s",
			"trigger": {
				"timeout": "1s",
				"period": "0.5s"
			},
			"parameters": [
				{"name": "AcquisitionMode",    "value": "Continuous"},
				{"name": "TriggerSelector",    "value": "FrameStart"},
				{"name": "TriggerMode",        "value": "On"},
				{"name": "TriggerSource",      "value": "Software"}
			]
		}
	]
}
`,
	},
}

// TestArray tests image acquisition with an array of cameras.
//
// This test supports the following command line flags:
//   - emu-only
//   - n-img
//
// TODO: should actually test whether we handle concurrency,
// thread locking, allocation/deallocation etc. properly.
func TestArray(t *testing.T) {
	for _, tt := range arrayTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			// configure the test
			if tt.NonEmulated && emuOnly {
				t.Skip("physical camera testing is disabled")
			}

			// setup
			p := struct {
				Cs Array               `json:"cameras"`
				IP *ocr.ImageProcessor `json:"image_processor"`
			}{
				IP: ocr.NewImageProcessor(),
			}
			defer func() {
				p.Cs.Delete()
				p.IP.Delete()
			}()
			// unmarshal
			err := json.Unmarshal([]byte(tt.Config), &p)
			assert.Nil(err, err)
			// initialize
			err = func() error {
				if err := p.Cs.Init(); err != nil {
					return err
				}
				if err := p.IP.Init(); err != nil {
					return err
				}
				return nil
			}()
			assert.Nil(err, err)

			// try to acquire images
			err = p.Cs.StartAcquisition()
			assert.Nil(err, err)
			defer p.Cs.StopAcquisition() // happens automatically

			var nAcquired uint64
			for p.Cs.IsAcquiring() && nAcquired < nReqImgs {
				t.Log("waiting for trigger...")
				err = p.Cs.TryTrigger()
				assert.Nil(err, err)
				t.Log("trigger fired")

				t.Log("acquiring...")
				err := p.Cs.Acquire()
				assert.Nil(err, err)
				for _, cam := range p.Cs {
					if cam.Result.Value != nil {
						nAcquired++
					}
				}
			}

			assert.ErrorIs(err, tt.Error, "unexpected error")
		})
	}
}
