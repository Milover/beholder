package neural

/*
#include "neural.h"
*/
import "C"
import (
	"fmt"

	"github.com/Milover/beholder/internal/mem"
)

func init() {
	RegisterNetwork(TypeYOLOv8, "yolov8", func() Network { return NewYOLOv8() })
}

const TypeYOLOv8 Type = 2 // YOLOv8 object detection models

// YOLOv8 is an object detection [Network] using the [YOLOv8 model].
//
// The [Network] interface is implemented by the embedded [network], so the
// same usage rules apply.
//
// WARNING: a new YOLOv8 should ALWAYS be created using [NewYOLOv8].
// WARNING: YOLOv8 contains C-managed resources so when it is no longer needed,
// [YOLOv8.Delete] must be called to release the resources and clean up.
//
// [YOLOv8 model]: https://docs.ultralytics.com/models/yolov8/
type YOLOv8 struct {
	// Classes are the friendly-names of object classes recognized by the model.
	// If not defined, (model-defined) class indexes are used during output.
	Classes []string `json:"classes"`

	network // the underlying network
}

// Init initializes the C-allocated API with the configuration data,
// if n is valid.
func (n *YOLOv8) Init() error {
	if err := n.IsValid(); err != nil {
		return fmt.Errorf("model.PARSeq.Init: %w", err)
	}
	if err := n.network.Init(); err != nil {
		return fmt.Errorf("model.PARSeq.Init: %w", err)
	}
	// configure YOLOv8-specific parameters
	ar := &mem.Arena{}
	defer ar.Free()

	ok := C.Det_ConfigureYOLOv8(
		n.p,
		(**C.char)(ar.CopyStrArray(n.Classes)),
		C.size_t(len(n.Classes)),
	)
	if !ok {
		return fmt.Errorf("network.YOLOv8.Init: %w", ErrInit)
	}
	return nil
}

// newYOLOv8CPtr is a helper function which wraps the C-call which allocates
// a new YOLOv8 C-API and returns a pointer to it.
func newYOLOv8CPtr() C.Det {
	return C.Det_NewYOLOv8()
}

// NewYOLOv8 constructs (C call) a new YOLOv8 object detection network.
// See the [docs] for more info about the default values.
// WARNING: [YOLOv8.Delete] must be called to release the memory
// when no longer needed.
//
// [docs]: https://docs.ultralytics.com/guides/preprocessing_annotated_data/#normalizing-pixel-values
func NewYOLOv8() *YOLOv8 {
	n := &YOLOv8{network: newNetwork()}
	n.p = newYOLOv8CPtr()

	n.Config.Scale = [3]float64{1.0 / 255.0, 1.0 / 255.0, 1.0 / 255.0}
	return n
}
