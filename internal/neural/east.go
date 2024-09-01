package neural

/*
#include "neural.h"
*/
import "C"

func init() {
	RegisterNetwork(TypeEAST, "east", func() Network { return NewEAST() })
}

const TypeEAST Type = 3 // EAST text detection models

// EAST is a text detection [Network] using the [EAST model].
//
// The [Network] interface is implemented by the embedded [network], so the
// same usage rules apply.
//
// WARNING: EAST contains C-managed resources so when it is no longer needed,
// [EAST.Delete] must be called to release the resources and clean up.
//
// FIXME: the bounding boxes are being incorrectly mapped from the blob
// back to the image
//
// [EAST model]: https://github.com/argman/EAST
type EAST struct {
	network // the underlying network
}

// newEASTCPtr is a helper function which wraps the C-call which allocates
// a new EAST C-API and returns a pointer to it.
func newEASTCPtr() C.Det {
	return C.Det_NewEAST()
}

// NewEAST constructs (C call) a new EAST text detection network.
// WARNING: [EAST.Delete] must be called to release the memory
// when no longer needed.
func NewEAST() *EAST {
	e := &EAST{network: newNetwork()}
	e.p = newEASTCPtr()
	// EAST expects a specific mean normalization value, see:
	// https://docs.opencv.org/4.10.0/d4/d43/tutorial_dnn_text_spotting.html
	e.Config.Mean = [3]float64{123.68, 116.78, 103.94}
	return e
}
