package camera

/*
#include <stdlib.h>
#include "camera.h"
*/
import "C"
import (
	"errors"
	"fmt"
	"time"
	"unsafe"

	"github.com/Milover/beholder/internal/chrono"
	"github.com/Milover/beholder/internal/mem"
)

// Parameter is a GenICam parameter.
type Parameter struct {
	// Name of the parameter.
	Name string `json:"name"`
	// Value of the parameter.
	Value string `json:"value"`
}

// Result represents the result of an image acquisition process.
type Result struct {
	// Value is the acquired image buffer.
	//
	// Value is a non-owning pointer to the buffer and does not require
	// explicit deallocation, it is managed by C.
	Value unsafe.Pointer
	// ID is the acquisition result id as asigned by the camera device.
	ID uint64
	// Timestamp is the time at which the result was acquired, i.e.
	// received by the host machine.
	Timestamp time.Time
}

const (
	// SNPickFirst is a special serial number value, which designates that
	// [Camera] should attach (connect to) the first camera device found
	// during initialization, instead of a specific one.
	SNPickFirst string = "pick-first"
)

// Camera represents the physical camera device.
//
// Camera needs to be initialized before use, after which point it should
// not be copied.
// WARNING: Camera holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
type Camera struct {
	// Type is the transport layer type used to communicate
	// with the camera device.
	//
	// The default type is 'GigE'.
	Type Type `json:"type"`

	// SN is the serial number of the camera device.
	// It is used to identify a physical camera device of the matching Type.
	//
	// During initialization, the transport layer is scanned for devices,
	// and if a camera device with a matching serial number is found,
	// it will be attached (connected to).
	// Initialization will fail if no device matching the specified serial
	// number is found.
	//
	// If multiple devices with a matching serial number are found,
	// the first one found will be attached. No guarantees are made as to
	// the order in which devices are found.
	//
	// SN can be set to a special value "pick-first", in which case, the
	// first device found will be attached. If no devices are found,
	// initialization will fail.
	// WARNING: this will not work properly when multiple cameras
	// of the same Type are used (the first device will be attached to
	// multiple Cameras), and is primarily meant to be used during testing.
	SN string `json:"serial_number"`

	// AcquisitionTimeout is the duration in which an image must be received
	// when calling [Acquire].
	AcquisitionTimeout chrono.Duration `json:"acquisition_timeout"`

	// Parameters is a key-value map of GenICam parameters to be forwarded
	// to the camera device.
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

// NewCamera constructs a new Camera with sensible defaults.
func NewCamera() *Camera {
	d, _ := time.ParseDuration("2s")
	return &Camera{
		Type:               GigE,
		AcquisitionTimeout: chrono.Duration{Duration: d},
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
// TODO: confusing, remove or refactor.
func (c Camera) IsAttached() bool {
	return bool(C.Cam_IsAttached(c.p))
}

// IsInitialized checks if the camera is ready to be used,
// eg. start image acquisition or set parameters.
// TODO: confusing, remove or refactor.
func (c Camera) IsInitialized() bool {
	return bool(C.Cam_IsInitialized(c.p))
}

// IsValid is function used as an assertion that c is able to be initialized.
func (c Camera) IsValid() error {
	if err := c.Type.IsValid(); err != nil {
		return fmt.Errorf("camera.Camera.IsValid: %w", err)
	}
	if len(c.SN) == 0 {
		return errors.New("camera.Camera.IsValid: camera serial number undefined")
	}
	if c.AcquisitionTimeout.Milliseconds() < 0 {
		return errors.New("camera.Camera.IsValid: bad image acquisition timeout")
	}
	// TODO: check parameters
	return nil
}

// Init initializes (C call) the camera device with configuration data,
// if c is valid.
//
// Once initialized, c should not be copied.
//
// WARNING: Delete must be called to release the memory when no longer needed.
func (c *Camera) Init() error {
	if err := c.IsValid(); err != nil {
		return err
	}
	// get the transport layer
	tl, err := getTransportLayer(c.Type)
	if err != nil {
		return err
	}
	// allocate C-memory for the camera
	c.p = C.Cam_New()
	if c.p == (C.Cam)(nil) {
		return errors.New("camera.Camera.Init: could not allocate C-memory")
	}
	ar := &mem.Arena{}
	defer ar.Free()

	// handle SN
	if c.SN == SNPickFirst {
		sn := ar.Store(unsafe.Pointer(C.Trans_GetFirstSN(tl.p)))
		if sn == nil {
			return errors.New("camera.Camera.Init: could not find a camera device")
		}
		c.SN = C.GoString((*C.char)(sn))
	}
	sn := (*C.char)(ar.CopyStr(c.SN))

	// handle parameters
	nPars := uint64(len(c.Parameters))
	pars := (*C.Par)(ar.Malloc(nPars * uint64(unsafe.Sizeof(C.Par{}))))
	parsSlice := unsafe.Slice(pars, nPars)
	for i, p := range c.Parameters {
		parsSlice[i].name = (*C.char)(ar.CopyStr(p.Name))
		parsSlice[i].value = (*C.char)(ar.CopyStr(p.Value))
	}
	if ok := C.Cam_Init(c.p, sn, pars, C.size_t(nPars), tl.p); !ok {
		return errors.New("camera.Camera.Init: could not initialize camera")
	}
	return nil
}

// StartAcquisition starts image acquisition on the camera device.
func (c Camera) StartAcquisition() error {
	if ok := C.Cam_StartAcquisition(c.p); !ok {
		return errors.New("camera.Camera.StartAcquisition: could not start image acquisition")
	}
	return nil
}

// StopAcquisition stops image acquisition on the camera device.
func (c Camera) StopAcquisition() {
	C.Cam_StopAcquisition(c.p)
}

// TryTrigger is a function that will try to execute a software trigger.
//
// If no trigger is available (defined) it returns nil immediately, otherwise
// it calls [Trigger.Execute] and returns the result.
func (c Camera) TryTrigger() error {
	if c.Trigger == nil {
		return nil
	}
	return c.Trigger.Execute(c)
}
