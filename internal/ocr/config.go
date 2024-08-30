package ocr

import (
	"errors"
	"fmt"
)

var (
	ErrConfig = errors.New("configuration error") // bad configuration
)

// Config holds [Network] specific configuration data.
type Config struct {
	// Size is the DNN input image size in pixels.
	//
	// The image is scaled and padded, via letter-boxing, so that the image
	// dimensions are Size*Size, while preserving the original aspect ratio.
	Size int `json:"size"`
	// Scale is a normalization constant by which pixel values are multiplied.
	Scale float64 `json:"scale"`
	// ConfidenceThreshold is the minimum confidence score needed to accept
	// a detected object.
	ConfidenceThreshold float32 `json:"confidence_threshold"`
	// NMSThreshold is the non-maximum suppression threshold.
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
		Size:                640,
		Scale:               1.0,
		ConfidenceThreshold: 0.5,
		NMSThreshold:        0.4,
		Mean:                [3]float64{0.0, 0.0, 0.0},
		SwapRB:              true,
		PadValue:            [3]float64{0.0, 0.0, 0.0},
	}
}

// IsValid is function used as an assertion that c has valid values.
func (c *Config) IsValid() error {
	if c.Size <= 0 {
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
