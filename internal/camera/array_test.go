// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package camera

import (
	"encoding/json"
	"errors"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
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
			require := require.New(t)

			// configure the test
			if tt.NonEmulated && emuOnly {
				t.Skip("physical camera testing is disabled")
			}

			// setup
			p := struct {
				Cs Array `json:"cameras"`
				//IP *imgproc.Processor `json:"image_processor"`
			}{
				//IP: imgproc.NewProcessor(),
			}
			defer func() {
				p.Cs.Delete()
				//p.IP.Delete()
			}()
			// unmarshal
			err := json.Unmarshal([]byte(tt.Config), &p)
			require.NoError(err)
			// initialize
			err = func() error {
				if err := p.Cs.Init(); err != nil {
					return err
				}
				//if err := p.IP.Init(); err != nil {
				//	return err
				//}
				return nil
			}()
			require.NoError(err)

			// try to acquire images
			err = p.Cs.StartAcquisition()
			require.NoError(err)
			defer p.Cs.StopAcquisition() // happens automatically

			for i := uint64(0); i < nImgs && p.Cs.IsAcquiring(); i++ {
				var err error

				t.Log("waiting for trigger...")
				err = errors.Join(err, p.Cs.TryTrigger())
				t.Log("trigger fired")

				t.Log("acquiring...")
				err = errors.Join(err, p.Cs.Acquire())
				for _, cam := range p.Cs {
					if cam.Result.Buffer != nil { //nolint:staticcheck // wat do?
					}
				}
				assert.ErrorIs(err, tt.Error, "unexpected error")
			}
		})
	}
}
