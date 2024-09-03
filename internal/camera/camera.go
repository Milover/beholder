//go:build linux

package camera

/*
#include <stdlib.h>
#include "camera.h"
*/
import "C"
import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"time"
	"unsafe"

	"github.com/Milover/beholder/internal/chrono"
	"github.com/Milover/beholder/internal/mem"
	"github.com/Milover/beholder/internal/models"
)

// Parameter is a GenICam parameter.
type Parameter struct {
	// Name of the parameter.
	Name string `json:"name"`
	// Value of the parameter.
	Value string `json:"value"`
}

// Parameters is a helper type representing a set of (GenICam) parameters.
type Parameters []Parameter

// makeCPars creates a C-array of parameters using ar for memory management.
//
// All C-memory allocated by makeCPars is stored into ar, including pars, i.e.
// the caller does not need to call [mem.Arena.Store] for pars.
// When the caller calls [mem.Arena.Free] on ar, all memory
// allocated by makeCPars is released.
func (p Parameters) makeCPars(ar *mem.Arena) (pars unsafe.Pointer, nPars uint64) {
	nPars = uint64(len(p))
	pars = ar.Malloc(nPars * uint64(unsafe.Sizeof(C.Par{})))
	parsSlice := unsafe.Slice((*C.Par)(pars), nPars)
	for i, par := range p {
		parsSlice[i].name = (*C.char)(ar.CopyStr(par.Name))
		parsSlice[i].value = (*C.char)(ar.CopyStr(par.Value))
	}
	return pars, nPars
}

const (
	// SNPickFirst is a special serial number value, which designates that
	// [Camera] should attach (connect to) the first camera device found
	// during initialization, instead of a specific one.
	SNPickFirst string = "pick-first"
)

var (
	ErrAcquisition = errors.New("image acquisition error")
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
	Parameters Parameters `json:"parameters"`

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
	// It is reset after each call to Acquire(), and has a non-nil
	// [models.Image.Buffer] only after successful acquisitions.
	Result models.Image `json:"-"`

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
	c.Result = models.Image{}
	ok := C.Cam_Acquire(c.p, (C.size_t)(c.AcquisitionTimeout.Milliseconds()))
	if !ok {
		return fmt.Errorf("camera.Camera.Acquire: %w", ErrAcquisition)
	}
	r := C.Cam_GetRawImage(c.p)
	if unsafe.Pointer(r.buffer) == nil {
		return fmt.Errorf("camera.Camera.Acquire: could not get raw image data")
	}
	c.Result = models.Image{
		Buffer:       unsafe.Pointer(r.buffer),
		ID:           uint64(r.id),
		Timestamp:    time.Now(),
		Rows:         int(r.rows),
		Cols:         int(r.cols),
		PixelType:    int64(r.pixelType),
		Step:         uint64(r.step),
		BitsPerPixel: uint64(r.bitsPerPixel),
	}
	return nil
}

// CmdExecute tries to execute a (GenICam) command.
//
// A non-nil error does not guarantee that a command was executed or that
// execution was successful.
func (c Camera) CmdExecute(command string) error {
	cmd := C.CString(command)
	defer C.free(unsafe.Pointer(cmd))
	if ok := C.Cam_CmdExecute(c.p, cmd); !ok {
		return fmt.Errorf("camera.Camera.CmdExecute: could not execute command: %q", command)
	}
	return nil
}

// CmdIsDone reports if a (GenICam) command finished executing.
//
// FIXME: we cannot differentiate between an error, eg. command unavailable,
// and execution not finishing.
func (c Camera) CmdIsDone(command string) bool {
	cmd := C.CString(command)
	defer C.free(unsafe.Pointer(cmd))
	return bool(C.Cam_CmdIsDone(c.p, cmd))
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
		sn := ar.StoreCStrConv(unsafe.Pointer(C.Trans_GetFirstSN(tl.p)))
		if len(sn) == 0 {
			return errors.New("camera.Camera.Init: could not find a camera device")
		}
		c.SN = sn
	}
	sn := (*C.char)(ar.CopyStr(c.SN))

	// handle parameters
	pars, nPars := c.Parameters.makeCPars(ar)

	if ok := C.Cam_Init(c.p, sn, (*C.Par)(pars), C.size_t(nPars), tl.p); !ok {
		return errors.New("camera.Camera.Init: could not initialize camera")
	}
	return nil
}

// SetParams sets (GenICam) parameters on the camera device.
func (c Camera) SetParameters(params Parameters) error {
	ar := &mem.Arena{}
	defer ar.Free()
	pars, nPars := params.makeCPars(ar)

	if ok := C.Cam_SetParameters(c.p, (*C.Par)(pars), C.size_t(nPars)); !ok {
		return errors.New("camera.Camera.SetParameters: could not set parameters")
	}
	return nil
}

// TstFailBuffers simulates a bad camera connection by generating
// failed buffers on an emulated camera device.
//
// The camera must be acquiring before calling TstFailBuffers.
//
// NOTE: this function works only with emulated cameras and
// is for internal use only.
func (c Camera) TstFailBuffers(count uint64) error {
	if c.Type != Emulated {
		return fmt.Errorf("camera.Camera.TstFailBuffers: non-emulated camera type: %q", c.Type)
	}
	if !c.IsAcquiring() {
		return errors.New("camera.Camera.TstFailBuffers: camera not acquiring")
	}
	err := c.SetParameters(Parameters{
		Parameter{Name: "ForceFailedBufferCount", Value: strconv.FormatUint(count, 10)},
	})
	if err != nil {
		return fmt.Errorf("camera.Camera.TstFailBuffers: %w", err)
	}
	if err := c.CmdExecute("ForceFailedBuffer"); err != nil {
		return fmt.Errorf("camera.Camera.TstFailBuffers: %w", err)
	}
	return nil
}

// TstSetImage sets the image file returned by an emulated camera device
// during acquisition.
//
// path can be a relative or absolute path to an image file or a directory
// containing image files, in which case the directory should only contain
// image files and no subdirectories.
//
// The image format must be either PNG or TIF. This is a limitation imposed by
// pylon.
//
// NOTE: this function works only with emulated cameras and is intended
// for internal use only.
//
// BUG: we need to also set camera ROI to the full image, otherwise the image
// gets clipped, i.e. something like:
//
//	err = c.SetParameters(Parameters{
//		Parameter{Name: "Width", Value: imgWidth},
//		Parameter{Name: "Height", Value: imgHeight},
//		Parameter{Name: "OffsetX", Value: "0"},
//		Parameter{Name: "OffsetY", Value: "0"},
//	})
//
// however, since we usually don't have info about the image until everything
// gets initialized, we might have to set/reset the ROI "manually" once the
// camera is attached.
func (c Camera) TstSetImage(path string) error {
	if c.Type != Emulated {
		return fmt.Errorf("camera.Camera.TstSetImage: non-emulated camera type: %q", c.Type)
	}
	absPath, err := filepath.Abs(path)
	if err != nil {
		return fmt.Errorf("camera.Camera.TstSetImage: %w", err)
	}
	if _, err := os.Stat(absPath); err != nil {
		return fmt.Errorf("camera.Camera.TstSetImage: %w", err)
	}
	// TODO: could check for subdirectories and/or image formats
	err = c.SetParameters(Parameters{
		Parameter{Name: "TestImageSelector", Value: "Off"}, // disable standard test images
		Parameter{Name: "ImageFileMode", Value: "On"},      // enable custom test images
		Parameter{Name: "ImageFilename", Value: absPath},   // load custom image(s) from disc
	})
	if err != nil {
		return fmt.Errorf("camera.Camera.TstSetImage: %w", err)
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
