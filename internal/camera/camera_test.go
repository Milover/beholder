// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package camera

import (
	_ "embed"
	"encoding/json"
	"errors"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

// TestCamera tests image acquisition with a single camera.
//
// This test supports the following command line flags:
//   - sn
//   - emu-only
//   - hw-trigger
//   - n-img
//   - cleanup
func TestCamera(t *testing.T) {
	tests := map[string]struct {
		Error          error
		Config         string
		NeedsHwTrigger bool // does the test need a hardware trigger?
		NonEmulated    bool // does the test use a physical camera device?
		FailBuffers    uint64
	}{
		"pick-first-emulated-w/-software-trigger": {
			Config: emulatedSWTriggerPickFirst,
		},
		"pick-first-emulated-w/-software-trigger-failed-buffers": {
			Config:      emulatedSWTriggerPickFirstFailedBuffers,
			FailBuffers: uint64(50),
			Error:       ErrAcquisition,
		},
		"single-emulated-w/-software-trigger": {
			Config: emulatedSWTriggerSingle,
		},
		"pick-first-gige-w/-software-trigger": {
			Config:      gigeSWTriggerPickFirst,
			NonEmulated: true,
		},
		"pick-first-gige-w/-hardware-trigger": {
			Config:         gigeHWTriggerPickFirst,
			NonEmulated:    true,
			NeedsHwTrigger: true,
		},
	}
	for name, tt := range tests {
		t.Run(name, func(t *testing.T) {
			assert := assert.New(t)
			require := require.New(t)
			// configure the test
			if tt.NonEmulated && emuOnly {
				t.Skip("physical camera testing is disabled")
			}
			if tt.NeedsHwTrigger && !hwTriggering {
				t.Skip("hardware triggering is disabled")
			}
			// setup
			p := struct {
				C *Camera `json:"camera"`
			}{
				C: NewCamera(),
			}
			defer func() {
				p.C.Delete()
			}()
			// unmarshal
			err := json.Unmarshal([]byte(tt.Config), &p)
			require.NoError(err)
			// initialize
			err = p.C.Init()
			require.NoError(err)

			// try to acquire images
			err = p.C.StartAcquisition()
			require.NoError(err)
			defer p.C.StopAcquisition() // happens automatically

			// generate acquisition errors
			if !tt.NonEmulated && tt.FailBuffers > uint64(0) {
				err = p.C.TstFailBuffers(tt.FailBuffers)
				require.NoError(err)
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
				if p.C.Result.Buffer != nil { //nolint:staticcheck // wat do?
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

//go:embed testdata/emulated_sw_trigger_pick_first.json
var emulatedSWTriggerPickFirst string

//go:embed testdata/emulated_sw_trigger_pick_first_failed_buffers.json
var emulatedSWTriggerPickFirstFailedBuffers string

//go:embed testdata/emulated_sw_trigger_single.json
var emulatedSWTriggerSingle string

//go:embed testdata/gige_sw_trigger_pick_first.json
var gigeSWTriggerPickFirst string

//go:embed testdata/gige_hw_trigger_pick_first.json
var gigeHWTriggerPickFirst string
