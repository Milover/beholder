package camera

/*
#include <stdlib.h>
#include "camera.h"
*/
import "C"
import (
	"fmt"
	"runtime"
	"sync"
	"unsafe"
)

// pylon is a handle to the pylon API runtime resource manager
//
// The pylon runtime must be initialized before any calls to the API,
// so this is ensured at the package level.
//
// Allocation and deallocation are handled with the init() function.
var pylon unsafe.Pointer

// tLayers are all transport layers used by the program during execution.
//
// [Camera] requires that a proper transport layer, corresponding to it's
// [Type], be available for it to be initialized. Since the transportLayer
// itself requires no configuration, and managing them is tedious and error
// prone, all transport layer management is done at the package level.
//
// Transport layers are initialized, as needed, during [Camera] initialization.
// Once initialized (allocated) they persist during the runtime of the program,
// and are freed once the program terminates.
//
// tLayers should never be accessed directly, if a transport layer is needed,
// use getTransportLayer.
var tLayers []transportLayer

// mTLayers synchronizes read/write access to tLayers.
var mTLayers sync.Mutex

// XXX: might leak or whatever, yolo
func init() {
	pylon = unsafe.Pointer(C.Pyl_New())
	// this is run after tLayers' finalizer because all transportLayers
	// point to pylon
	runtime.SetFinalizer(&pylon, func(p *unsafe.Pointer) {
		C.Pyl_Delete((*C.Pyl)(p))
	})
	runtime.SetFinalizer(&tLayers, func(tls *[]transportLayer) {
		for _, tl := range *tls {
			tl.Delete()
		}
		clear(*tls)
	})
}

// transportLayer is a transport layer used to communicate with camera devices.
//
// WARNING: transportLayer holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
type transportLayer struct {
	// typ is the type of camera device with which the transport layer
	// can communicate.
	typ Type
	// p is a pointer to the C-allocated transport layer.
	p C.Trans
	// api is a (weak) pointer to the pylon runtime resource manager.
	//
	// The pylon API must be present during the lifetime of any pylon API
	// resources, hence the purpose of api is to ensure the correct execution
	// order of finalizers.
	api *unsafe.Pointer
}

// getTransportLayer is a function which returns a pointer to a transport layer
// of type typ if one is available, otherwise it initializes a new
// transport layer (C call) and returns a pointer to it.
//
// It is safe to call getTransportLayer from multiple goroutines.
func getTransportLayer(typ Type) (*transportLayer, error) {
	mTLayers.Lock()
	defer mTLayers.Unlock()

	// check if an appropriate transport layer is available
	for i := range tLayers {
		if tLayers[i].typ == typ {
			return &tLayers[i], nil
		}
	}
	// if an appropriate transport layer is not available, create a new one
	tl := transportLayer{
		typ: typ,
		p:   C.Trans_New(),
		api: &pylon,
	}
	if ok := C.Trans_Init(tl.p, C.int(typ)); !ok {
		tl.Delete()
		return nil, fmt.Errorf("camera.transportLayer.Init: could not initialize transport layer of type: %q", typ)
	}
	tLayers = append(tLayers, tl)
	return &tl, nil
}

// Delete releases C-allocated memory. Once called, tl is no longer valid.
// If tl is the last transport layer instance, the pylon runtime resources
// are also freed.
func (tl *transportLayer) Delete() {
	C.Trans_Delete((*C.Trans)(&tl.p))
}
