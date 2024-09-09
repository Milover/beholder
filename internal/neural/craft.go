package neural

/*
#include "neural.h"
*/
import "C"

func init() {
	RegisterNetwork(TypeCRAFT, "craft", func() Network { return NewCRAFT() })
}

const TypeCRAFT Type = 4 // CRAFT text detection models

// CRAFT is a text detection [Network] using the [CRAFT] model.
//
// The [Network] interface is implemented by the embedded [network], so the
// same usage rules apply.
//
// WARNING: CRAFT contains C-managed resources so when it is no longer needed,
// [CRAFT.Delete] must be called to release the resources and clean up.
//
// [CRAFT]: https://github.com/clovaai/CRAFT-pytorch
type CRAFT struct {
	TextThreshold float32 // text confidence threshold
	LinkThreshold float32 // link confidence threshold
	LowText       float32 // text low-bound score

	network // the underlying network
}

// newCRAFTPtr is a helper function which wraps the C-call which allocates
// a new CRAFT C-API and returns a pointer to it.
func newCRAFTPtr() C.Det {
	return C.Det_NewCRAFT()
}

// NewCRAFT constructs (C call) a new CRAFT text detection network.
// WARNING: [CRAFT.Delete] must be called to release the memory
// when no longer needed.
func NewCRAFT() *CRAFT {
	y := &CRAFT{
		TextThreshold: 0.7,
		LinkThreshold: 0.4,
		LowText:       0.4,
		network:       newNetwork(),
	}
	y.p = newCRAFTPtr()
	// CRAFT expects normalized values, see:
	// https://github.com/clovaai/CRAFT-pytorch/blob/e332dd8b718e291f51b66ff8f9ef2c98ee4474c8/imgproc.py#L20
	y.Config.Scale = [3]float64{
		1.0 / (0.229 * 255.0),
		1.0 / (0.224 * 255.0),
		1.0 / (0.225 * 255.0),
	}
	y.Config.Mean = [3]float64{
		0.485 / 255.0,
		0.456 / 255.0,
		0.406 / 255.0,
	}
	return y
}
