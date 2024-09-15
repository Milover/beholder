package neural

/*
#include "neural.h"
*/
import "C"
import (
	"errors"
	"fmt"
)

func init() {
	RegisterNetwork(TypeCRAFT, "craft", func() Network { return NewCRAFT() })
}

const TypeCRAFT Type = 4 // CRAFT text detection models

// CRAFT is a text detection [Network] using the [CRAFT model].
//
// The [Network] interface is implemented by the embedded [network], so the
// same usage rules apply.
//
// WARNING: a new CRAFT should ALWAYS be created using [NewCRAFT].
// WARNING: CRAFT contains C-managed resources so when it is no longer needed,
// [CRAFT.Delete] must be called to release the resources and clean up.
//
// [CRAFT model]: https://github.com/clovaai/CRAFT-pytorch
type CRAFT struct {
	TextThreshold float32 // text confidence threshold
	LinkThreshold float32 // link confidence threshold
	LowText       float32 // text low-bound score

	network // the underlying network
}

// Init initializes the C-allocated API with the configuration data,
// if n is valid.
func (n *CRAFT) Init() error {
	if err := n.IsValid(); err != nil {
		return fmt.Errorf("model.CRAFT.Init: %w", err)
	}
	if err := n.network.Init(); err != nil {
		return fmt.Errorf("model.CRAFT.Init: %w", err)
	}
	// configure CRAFT-specific parameters
	ok := C.Det_ConfigureCRAFT(
		n.p,
		C.float(n.TextThreshold),
		C.float(n.LinkThreshold),
		C.float(n.LowText),
	)
	if !ok {
		return fmt.Errorf("network.CRAFT.Init: %w", ErrInit)
	}
	return nil
}

// IsValid asserts that n can be initialized.
func (n *CRAFT) IsValid() error {
	if err := n.network.IsValid(); err != nil {
		return fmt.Errorf("model.CRAFT.IsValid: %w", err)
	}
	if n.TextThreshold < 0.0 || n.TextThreshold > 1.0 {
		return errors.New("network.CRAFT.IsValid: bad text threshold")
	}
	if n.LinkThreshold < 0.0 || n.LinkThreshold > 1.0 {
		return errors.New("network.CRAFT.IsValid: bad link threshold")
	}
	if n.LowText < 0.0 || n.LowText > 1.0 {
		return errors.New("network.CRAFT.IsValid: bad text low-bound")
	}
	return nil
}

// newCRAFTPtr is a helper function which wraps the C-call which allocates
// a new CRAFT C-API and returns a pointer to it.
func newCRAFTCPtr() C.Det {
	return C.Det_NewCRAFT()
}

// NewCRAFT constructs (C call) a new CRAFT text detection network.
// See the [docs] for more info about the default values.
// WARNING: [CRAFT.Delete] must be called to release the memory
// when no longer needed.
//
// [docs]: https://github.com/clovaai/CRAFT-pytorch/blob/e332dd8b718e291f51b66ff8f9ef2c98ee4474c8/imgproc.py#L20
func NewCRAFT() *CRAFT {
	n := &CRAFT{
		TextThreshold: 0.7,
		LinkThreshold: 0.4,
		LowText:       0.4,
		network:       newNetwork(),
	}
	n.p = newCRAFTCPtr()

	n.Config.Scale = [3]float64{
		1.0 / (0.229 * 255.0),
		1.0 / (0.224 * 255.0),
		1.0 / (0.225 * 255.0),
	}
	n.Config.Mean = [3]float64{
		0.485 / 255.0,
		0.456 / 255.0,
		0.406 / 255.0,
	}
	return n
}
