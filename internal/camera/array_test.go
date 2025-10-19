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

// TestArray tests image acquisition with an array of cameras.
//
// This test supports the following command line flags:
//   - emu-only
//   - n-img
//
// TODO: should actually test whether we handle concurrency,
// thread locking, allocation/deallocation etc. properly.
func TestArray(t *testing.T) {
	tests := map[string]struct {
		Error       error
		Config      string
		NonEmulated bool // does the test use a physical camera device?
	}{
		"emulated-w/-software-trigger": {
			Config: arrayEmulatedSWTrigger,
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
			// setup
			p := struct {
				Cs Array `json:"cameras"`
			}{}
			defer func() {
				p.Cs.Delete()
			}()
			// unmarshal
			err := json.Unmarshal([]byte(tt.Config), &p)
			require.NoError(err)
			// initialize
			err = p.Cs.Init()
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

//go:embed testdata/array.emulated_sw_trigger.json
var arrayEmulatedSWTrigger string
