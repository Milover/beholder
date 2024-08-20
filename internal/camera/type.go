//go:build linux

package camera

import (
	"errors"

	"github.com/Milover/beholder/internal/enumutils"
)

// Type is the transport layer type used to communicate with
// a camera device.
type Type int64

const (
	GigE Type = iota
	Emulated
)

var (
	typeMap = map[Type]string{
		GigE:     "gige",
		Emulated: "emulated",
	}
	invTypeMap = enumutils.Invert(typeMap)
)

func (t *Type) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, t, invTypeMap)
}

func (t Type) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(t, typeMap)
}

func (t Type) String() string {
	switch t {
	case GigE:
		return "gige"
	case Emulated:
		return "emulated"
	default:
		return "unknown"
	}
}

// IsValid is function used as an assertion that t is one of
// the predefined camera device types.
func (t Type) IsValid() error {
	switch t {
	case GigE, Emulated:
		return nil
	default:
		return errors.New("camera.Camera.IsValid: camera serial number undefined")

	}
}
