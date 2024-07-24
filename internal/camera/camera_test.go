package camera

import (
	"encoding/json"
	"fmt"
	"path"
	"strconv"
	"testing"

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

// Test the pylon API
type pylonTest struct {
	Name   string
	Error  error
	Config string
}

var pylonTests = []pylonTest{
	{
		Name:  "basic",
		Error: nil,
		Config: `
{
	"camera": {
		"mac": "00:30:53:2F:3F:8C",
		"parameters": [
			{"name": "AcquisitionMode",          "value": "Continuous"},
			{"name": "TriggerMode",              "value": "On"},
			{"name": "TriggerSource",            "value": "Line1"},
			{"name": "TriggerActivation",        "value": "RisingEdge"},
			{"name": "LineSelector",             "value": "Line2"},
			{"name": "LineMode",                 "value": "Output"},
			{"name": "LineSource",               "value": "FrameTriggerWait"},
			{"name": "LineInverter",             "value": "true"},
			{"name": "ExposureMode",             "value": "Timed"},
			{"name": "ExposureTimeAbs",          "value": "10000"},
			{"name": "ChunkModeActive",          "value": "true"},
			{"name": "ChunkSelector",            "value": "PayloadCRC16"},
			{"name": "ChunkEnable",              "value": "true"}
		]
	}
}
`,
	},
}

func TestPylon(t *testing.T) {
	for _, tt := range pylonTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)
			outdir := t.TempDir()

			// setup
			p := NewPylon()
			defer p.Finalize()
			// unmarshal
			err := json.Unmarshal([]byte(tt.Config), &p)
			assert.Nil(err, err)
			// initialize
			err = p.Init()
			assert.Nil(err, err)

			// try to acquire images
			err = p.C.StartAcquisition()
			assert.Nil(err, err)

			nAcquired := 0
			for p.C.IsAcquiring() {
				fmt.Println("acquiring...")

				img, err := p.C.Acquire()
				assert.Nil(err, err)
				if img == nil {
					continue
				}
				fmt.Println("img ID: ", img.ID)

				// try to output the image
				err = img.Write(path.Join(outdir, "img_"+strconv.FormatUint(img.ID, 10)+".png"))
				assert.Nil(err, err)

				img.Delete()

				nAcquired++
				if nAcquired >= 5 {
					p.C.StopAcquisition()
				}
			}

			assert.ErrorIs(err, tt.Error, "unexpected error")
		})
	}
}
