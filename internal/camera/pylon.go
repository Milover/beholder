package camera

/*
#include <stdlib.h>
#include "camera.h"
*/
import "C"

// Pylon is a handle to the pylon API runtime resource manager.
//
// WARNING: a Pylon instance must be present before any calls to the
// pylon-API.
// WARNING: Pylon holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
type Pylon struct {
	TL *TransportLayer `json:"transport_layer"`
	C  *Camera         `json:"camera"`

	p C.Pyl
}

// newPylon constructs (C call) a new pylon API manager.
// WARNING: since Pylon's fields allocate C-memory,
// Delete (or Finalize) must be called to release memory when no longer needed.
func NewPylon() Pylon {
	return Pylon{
		TL: NewTransportLayer(),
		C:  NewCamera(),
		p:  C.Pyl_New(),
	}
}

// Delete releases C-allocated memory by calling each field's Delete method.
// Once called, the contents (fields) of p are no longer valid.
func (p *Pylon) Delete() {
	p.C.Delete()
	p.TL.Delete()
}

// Finalize frees all of p's resources.
// Once called, p is no longer valid.
func (p *Pylon) Finalize() error {
	p.Delete()
	C.Pyl_Delete((*C.Pyl)(&p.p))
	return nil
}

// Init initializes C-allocated APIs with configuration data by calling
// each of p's fields Init method.
func (p Pylon) Init() error {
	if err := p.TL.Init(); err != nil {
		return err
	}
	if err := p.C.Init(*p.TL); err != nil {
		return err
	}
	return nil
}
