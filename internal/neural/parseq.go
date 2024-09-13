package neural

/*
#include "neural.h"
*/
import "C"

func init() {
	RegisterNetwork(TypePARSeq, "parseq", func() Network { return NewPARSeq() })
}

const TypePARSeq Type = 5 // PARSeq text recognition models

// PARSeq is a text recognition [Network] using the [PARSeq model].
//
// The current implementation uses a non-autoregressive encoder, with greedy
// decoding and no refinement iterations. These are OpenCV imposed constraints.
//
// The [Network] interface is implemented by the embedded [network], so the
// same usage rules apply.
//
// WARNING: PARSeq contains C-managed resources so when it is no longer needed,
// [PARSeq.Delete] must be called to release the resources and clean up.
//
// FIXME: forward runtime params to C-API
//
// [PARSeq model]: https://github.com/baudm/parseq
type PARSeq struct {
	Charset string // the character set recognized by the model

	network // the underlying network
}

// newEASTCPtr is a helper function which wraps the C-call which allocates
// a new EAST C-API and returns a pointer to it.
func newPARSeqPtr() C.Det {
	return C.Det_NewPARSeq()
}

// NewPARSeq constructs (C call) a new PARSeq text recognition network.
// WARNING: [PARSeq.Delete] must be called to release the memory
// when no longer needed.
func NewPARSeq() *PARSeq {
	n := &PARSeq{network: newNetwork()}
	n.p = newPARSeqPtr()
	n.Charset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~)"
	// PARSeq expects normalized values, see:
	// https://github.com/baudm/parseq/blob/1902db043c029a7e03a3818c616c06600af574be/strhub/data/module.py#L69
	n.Config.Scale = [3]float64{0.5 / 255.0, 0.5 / 255.0, 0.5 / 255.0}
	n.Config.Mean = [3]float64{127.5, 127.5, 127.5}
	return n
}
