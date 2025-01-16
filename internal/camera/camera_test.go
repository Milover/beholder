// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

//go:build linux

package camera

import (
	"encoding/json"
	"errors"
	"testing"

	"github.com/stretchr/testify/assert"
)

type cameraTest struct {
	Name           string
	Error          error
	Config         string
	NeedsHwTrigger bool // does the test need a hardware trigger?
	NonEmulated    bool // does the test use a physical camera device?
	FailBuffers    uint64
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
			{"name": "TriggerSource",      "value": "Software"}
		]
	}
}
`,
	},
	{
		Name:        "pick-first-emulated-w/-software-trigger-failed-buffers",
		Error:       ErrAcquisition,
		FailBuffers: uint64(50),
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
			{"name": "TriggerSource",      "value": "Software"}
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

// TestCamera tests image acquisition with a single camera.
//
// This test supports the following command line flags:
//   - sn
//   - emu-only
//   - hw-trigger
//   - n-img
//   - cleanup
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
			/*
				var outDir string
				if cleanUp {
					outDir = t.TempDir()
				}
			*/

			// setup
			p := struct {
				C *Camera `json:"camera"`
				//IP *imgproc.Processor `json:"image_processor"`
			}{
				C: NewCamera(),
				//IP: imgproc.NewProcessor(),
			}
			defer func() {
				p.C.Delete()
				//p.IP.Delete()
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
				//if err := p.IP.Init(); err != nil {
				//	return err
				//}
				return nil
			}()
			assert.Nil(err, err)

			// try to acquire images
			err = p.C.StartAcquisition()
			assert.Nil(err, err)
			defer p.C.StopAcquisition() // happens automatically

			// generate acquisition errors
			if !tt.NonEmulated && tt.FailBuffers > uint64(0) {
				err = p.C.TstFailBuffers(tt.FailBuffers)
				assert.Nil(err, err)
			}

			for i := uint64(0); i < nImgs && p.C.IsAcquiring(); i++ {
				var err error

				if !tt.NeedsHwTrigger {
					t.Log("waiting for trigger...")
					err = errors.Join(err, p.C.TryTrigger())
					t.Log("trigger fired")
				}
				t.Log("acquiring...")
				err = errors.Join(err, p.C.Acquire())

				// TODO: should be in a separate test probably
				if p.C.Result.Buffer != nil {
					// wat do?
					/*
						t.Log("writing...")
						filename := fmt.Sprintf("img_%v_%v.png",
							p.C.Result.Timestamp.Format("2006-01-02_15-04-05"),
							strconv.FormatUint(p.C.Result.ID, 10))
						err = errors.Join(
							err,
							p.IP.WriteAcquisitionResult(
								p.C.Result.Buffer, path.Join(outDir, filename),
							))
					*/
				}
				assert.ErrorIs(err, tt.Error, "unexpected error")
			}
		})
	}
}
