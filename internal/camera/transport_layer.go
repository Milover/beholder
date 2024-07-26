package camera

/*
#include <stdlib.h>
#include "camera.h"
*/
import "C"
import "errors"

// TransportLayer represents the transport layer used to communicate with
// the camera.
//
// WARNING: Camera depends on the TransportLayer being available to
// function properly, so it must be available during it's lifetime.
// WARNING: TransportLayer holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
type TransportLayer struct {
	p C.Trans
}

// NewTransportLayer constructs (C call) a new transport layer with sensible
// defaults.
// WARNING: Delete must be called to release the memory when no longer needed.
func NewTransportLayer() *TransportLayer {
	return &TransportLayer{
		p: C.Trans_New(),
	}
}

// Init initializes the C-allocated API with the configuration data,
// if tl is valid.
func (tl TransportLayer) Init() error {
	if ok := C.Trans_Init(tl.p); !ok {
		return errors.New("camera.TransportLayer.Init: could not initialize transport layer")
	}
	return nil
}

// Delete releases C-allocated memory. Once called, tl is no longer valid.
func (tl *TransportLayer) Delete() {
	C.Trans_Delete((*C.Trans)(&tl.p))
}
