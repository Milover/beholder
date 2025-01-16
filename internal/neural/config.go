// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package neural

import (
	"errors"
	"fmt"
	"slices"

	"github.com/Milover/beholder/internal/veryprecise"
)

var (
	ErrConfig = errors.New("configuration error") // bad configuration
)

// Config holds [Network] specific configuration data.
type Config struct {
	// Size is the DNN input image width x height in pixels.
	//
	// The image is scaled and padded, via letter-boxing, so that the image
	// dimensions are (W × H) Size[0] × Size[1], while optionally preserving
	// the original aspect ratio.
	//
	// TODO: this need a better explanation.
	Size [2]int `json:"size"`
	// Scale is a normalization constant by which pixel values are multiplied.
	Scale [3]float64 `json:"scale"`
	// ConfidenceThreshold is the minimum confidence score needed to accept
	// a detected object.
	ConfidenceThreshold float32 `json:"confidence_threshold"`
	// NMSThreshold is the non-maximum suppression threshold.
	//
	// TODO: should be moved somewhere else, since only some models use NMS
	NMSThreshold float32 `json:"nms_threshold"`
	// Mean is a normalization constant which is subtracted from pixel values.
	Mean [3]float64 `json:"mean"`
	// SwapRB determines if the red and green channels of the image are swapped.
	SwapRB bool `json:"swap_rb"`
	// PadValue is the pixel value used to pad the image.
	PadValue [3]float64 `json:"pad_value"`
}

// NewConfig returns a new Config with default values.
func NewConfig() *Config {
	return &Config{
		Size:                [2]int{640, 640},
		Scale:               [3]float64{1.0, 1.0, 1.0},
		ConfidenceThreshold: 0.5,
		NMSThreshold:        0.4,
		Mean:                [3]float64{0.0, 0.0, 0.0},
		SwapRB:              true,
		PadValue:            [3]float64{0.0, 0.0, 0.0},
	}
}

// Equal reports whether two Configs are equal.
// Floating-point values are considered equal if the difference is within 2 ULPs.
func (c *Config) Equal(c1 *Config) bool {
	if c == c1 {
		return true
	}
	ulp := 2
	equal32 := func(x, y float32) bool {
		return veryprecise.EqualULP32(x, y, ulp)
	}
	equal64 := func(x, y float64) bool {
		return veryprecise.EqualULP(x, y, ulp)
	}
	return slices.Equal(c.Size[:], c1.Size[:]) &&
		slices.EqualFunc(c.Scale[:], c1.Scale[:], equal64) &&
		slices.EqualFunc(c.Mean[:], c1.Mean[:], equal64) &&
		slices.EqualFunc(c.PadValue[:], c1.PadValue[:], equal64) &&
		equal32(c.ConfidenceThreshold, c1.ConfidenceThreshold) &&
		equal32(c.NMSThreshold, c1.NMSThreshold) &&
		c.SwapRB == c1.SwapRB
}

// IsValid is function used as an assertion that c has valid values.
func (c *Config) IsValid() error {
	if c.Size[0] <= 0 || c.Size[1] <= 0 {
		return fmt.Errorf("%w: bad size", ErrConfig)
	}
	if c.ConfidenceThreshold < 0.0 || c.ConfidenceThreshold > 1.0 {
		return fmt.Errorf("%w: bad confidence threshold", ErrConfig)
	}
	if c.NMSThreshold < 0.0 || c.NMSThreshold > 1.0 {
		return fmt.Errorf("%w: bad non-maximum suppression threshold", ErrConfig)
	}
	// TODO: not sure if we have to check the other stuff
	return nil
}
