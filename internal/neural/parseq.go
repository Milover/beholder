package neural

/*
#include "neural.h"
*/
import "C"
import (
	"errors"
	"fmt"

	"github.com/Milover/beholder/internal/mem"
)

func init() {
	RegisterNetwork(TypePARSeq, "parseq", func() Network { return NewPARSeq() })
}

const TypePARSeq Type = 5 // PARSeq text recognition models

// PARSeq is a text recognition [Network] using the [PARSeq model].
//
// The current implementation uses a non-autoregressive encoder with greedy
// decoding and no refinement iterations. These are [OpenCV imposed constraints].
//
// The [Network] interface is implemented by the embedded [network], so the
// same usage rules apply.
//
// WARNING: a new PARSeq should ALWAYS be created using [NewPARSeq].
// WARNING: PARSeq contains C-managed resources so when it is no longer needed,
// [PARSeq.Delete] must be called to release the resources and clean up.
//
// [PARSeq model]: https://github.com/baudm/parseq
// [OpenCV imposed constraints]: https://github.com/opencv/opencv/wiki/FAQ#dnn
type PARSeq struct {
	// TODO: special type so that we can use (unmarshal) a keyword in the
	// runtime config referring to a default charset
	Charset string // the character set recognized by the model

	network // the underlying network
}

// Init initializes the C-allocated API with the configuration data,
// if n is valid.
func (n *PARSeq) Init() error {
	if err := n.IsValid(); err != nil {
		return fmt.Errorf("model.PARSeq.Init: %w", err)
	}
	if err := n.network.Init(); err != nil {
		return fmt.Errorf("model.PARSeq.Init: %w", err)
	}
	// configure PARSeq-specific parameters
	ar := &mem.Arena{}
	defer ar.Free()

	ok := C.Det_ConfigurePARSeq(n.p, (*C.char)(ar.CopyStr(n.Charset)))
	if !ok {
		return fmt.Errorf("network.PARSeq.Init: %w", ErrInit)
	}
	return nil
}

// IsValid asserts that n can be initialized.
func (n *PARSeq) IsValid() error {
	if err := n.network.IsValid(); err != nil {
		return fmt.Errorf("model.PARSeq.IsValid: %w", err)
	}
	if len(n.Charset) == 0 {
		return errors.New("network.PARSeq.IsValid: charset empty")
	}
	return nil
}

// newPARSeqCPtr is a helper function which wraps the C-call which allocates
// a new PARSeq C-API and returns a pointer to it.
func newPARSeqCPtr() C.Det {
	return C.Det_NewPARSeq()
}

// NewPARSeq constructs (C call) a new PARSeq text recognition network.
// See the [docs] for more info about the default values.
// WARNING: [PARSeq.Delete] must be called to release the memory
// when no longer needed.
//
// [docs]: https://github.com/baudm/parseq/blob/1902db043c029a7e03a3818c616c06600af574be/strhub/data/module.py#L69
func NewPARSeq() *PARSeq {
	n := &PARSeq{
		Charset: "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",
		network: newNetwork(),
	}
	n.p = newPARSeqCPtr()

	n.Config.Scale = [3]float64{0.5 / 255.0, 0.5 / 255.0, 0.5 / 255.0}
	n.Config.Mean = [3]float64{127.5, 127.5, 127.5}
	return n
}
