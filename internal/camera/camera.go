package camera

/*
#include <stdlib.h>
#include "camera.h"
*/
import "C"
import (
	"errors"
	"regexp"
	"runtime"
	"strings"
	"time"
	"unsafe"

	"github.com/Milover/beholder/internal/chrono"
)

// pylon is a handle to the pylon API runtime resource manager.
//
// The pylon runtime needs to be initialized before any calls to the API,
// so this is ensured at the package level.
var pylon unsafe.Pointer

func init() {
	pylon = unsafe.Pointer(C.Pyl_New())
	// XXX: might leak or whatever, yolo
	runtime.SetFinalizer(&pylon, func(p *unsafe.Pointer) {
		C.Pyl_Delete((*C.Pyl)(p))
	})
}

var validMAC = regexp.MustCompile(`^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$|^([0-9A-Fa-f]{12})$`)

// IsValidMAC verifies a MAC addresss.
func IsValidMAC(addr string) bool {
	return validMAC.MatchString(addr)
}

// Parameter is a GenICam parameter.
type Parameter struct {
	// Name of the parameter.
	Name string `json:"name"`
	// Value of the parameter.
	Value string `json:"value"`
}

type Result struct {
	// Value is the acquisition result value.
	Value unsafe.Pointer
	// ID is the acquisition result id as asigned by the camera device.
	ID uint64
	// Timestamp is the time at which the result was acquired, i.e.
	// received by the host machine.
	Timestamp time.Time
}

// Camera represents the physical camera device.
//
// WARNING: Camera holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
type Camera struct {
	// MAC is the MAC address of the physical camera device.
	MAC string `json:"mac"`
	// Timeout is the image acquisition timeout
	AcquisitionTimeout chrono.Duration `json:"acquisition_timeout"`
	// Params is a key-value map of GenICam parameters to be forwarded
	// to the camera device.
	// FIXME: no good, need a slice of JSON unmarshallable type which
	Parameters []Parameter `json:"parameters"`
	// Trigger is an optional field which defines if and when a software
	// trigger signal should be sent to the camera device.
	// Note that the camera device needs to be properly set up for the trigger
	// signal to have an effect, for example:
	//
	//	TriggerSelector = FrameStart;
	//	TriggerMode     = On;
	//	TriggerSource   = Software;
	Trigger *Trigger `json:"trigger"`
	// Result is the acquisition result.
	// It is reset after each call to Acquire(), and has a non-nil Value only
	// after successful acquisitions.
	Result Result `json:"-"`

	p C.Cam
}

// NewCamera constructs (C call) a new camera device with sensible defaults.
// WARNING: Delete must be called to release the memory when no longer needed.
func NewCamera() *Camera {
	d, _ := time.ParseDuration("2s")
	return &Camera{
		AcquisitionTimeout: chrono.Duration{Duration: d},
		p:                  C.Cam_New(),
	}
}

// Acquire attempts to acquire an image from the camera.
//
// NOTE: if a recoverable error occurs, both the returned Image and the error
// will be nil, however if an unrecoverable error occurs, the returned Image
// will be nil, but the error will be non-nil.
// In this context, a recoverable error is one that allows for image acquisition
// to continue, eg. acquisition timeout or CRC check failure, while
// an unrecoverable error is one which requires explicit handling,
// eg. the camera device was detached.
func (c *Camera) Acquire() error {
	c.Result = Result{}
	ok := C.Cam_Acquire(c.p, (C.size_t)(c.AcquisitionTimeout.Milliseconds()))
	if !ok {
		return errors.New("camera.Camera.Acquire: image acquisition failed")
	}
	r := C.Cam_GetResult(c.p)
	c.Result = Result{
		Value:     unsafe.Pointer(r.ptr),
		ID:        uint64(r.id),
		Timestamp: time.Now(),
	}
	return nil
}

// Delete releases C-allocated memory. Once called, c is no longer valid.
func (c *Camera) Delete() {
	C.Cam_Delete((*C.Cam)(&c.p))
}

// IsAcquiring reports the image acquisition status of the camera.
func (c Camera) IsAcquiring() bool {
	return bool(C.Cam_IsAcquiring(c.p))
}

// IsAttached checks if a camera device is attached.
func (c Camera) IsAttached() bool {
	return bool(C.Cam_IsAttached(c.p))
}

// IsInitialized checks if the camera is ready to be used,
// eg. start image acquisition or set parameters.
func (c Camera) IsInitialized() bool {
	return bool(C.Cam_IsInitialized(c.p))
}

// IsValid is function used as an assertion that c is able to be initialized.
func (c Camera) IsValid() error {
	if c.p == (C.Cam)(nil) {
		return errors.New("camera.Camera.IsValid: nil camera pointer")
	}
	if !IsValidMAC(c.MAC) {
		return errors.New("camera.Camera.IsValid: bad MAC address")
	}
	if c.AcquisitionTimeout.Milliseconds() < 0 {
		return errors.New("camera.Camera.IsValid: bad image acquisition timeout")
	}
	// TODO: check parameters
	return nil
}

// Init initializes the C-allocated API with the configuration data,
// if c is valid.
func (c Camera) Init(tl TransportLayer) error {
	if err := c.IsValid(); err != nil {
		return err
	}

	// handle MAC address
	addr := C.CString(strings.NewReplacer(":", "", "-", "").Replace(c.MAC))
	defer C.free(unsafe.Pointer(addr))

	// handle parameters
	parSize := unsafe.Sizeof(C.Par{})
	nPars := C.size_t(len(c.Parameters))
	pars := (*C.Par)(C.malloc(nPars * C.size_t(parSize)))
	defer C.free(unsafe.Pointer(pars))
	parsSlice := unsafe.Slice(pars, int(nPars))
	iPar := 0
	for _, p := range c.Parameters {
		parsSlice[iPar].name = C.CString(p.Name)
		defer C.free(unsafe.Pointer(parsSlice[iPar].name))
		parsSlice[iPar].value = C.CString(p.Value)
		defer C.free(unsafe.Pointer(parsSlice[iPar].value))
		iPar++
	}

	if ok := C.Cam_Init(c.p, addr, pars, nPars, tl.p); !ok {
		return errors.New("camera.Camera.Init: could not initialize camera")
	}
	return nil
}

// StartAcquisition starts image acquisition on the camera.
func (c Camera) StartAcquisition() error {
	if ok := C.Cam_StartAcquisition(c.p); !ok {
		return errors.New("camera.Camera.StartAcquisition: could not start image acquisition")
	}
	return nil
}

// StopAcquisition stops image acquisition on the camera.
func (c Camera) StopAcquisition() {
	C.Cam_StopAcquisition(c.p)
}

// TryTrigger will to execute a software trigger if it is available.
// If no trigger is available (defined) returns nil immediately, otherwise
// calls Trigger.Execute(...) and returns the result.
func (c Camera) TryTrigger() error {
	if c.Trigger == nil {
		return nil
	}
	return c.Trigger.Execute(c)
}
