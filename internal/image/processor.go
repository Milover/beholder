package image

/*
#include <stdlib.h>
#include "image.h"
*/
import "C"
import (
	"encoding/json"
	"errors"
	"fmt"
	"unsafe"

	"github.com/Milover/beholder/internal/neutral"
)

type ReadMode int

const (
	RMUnchanged ReadMode = iota - 1
	RMGrayscale
	RMAnyDepth
	RMAnyColor = iota
)

// Processor is a handle for the image processing API
// and contains API configuration data.
// WARNING: Processor holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
// TODO: missing a way to reset the image/read an image from a []byte.
type Processor struct {
	// Postprocessing holds a list of configurations for
	// image postprocessing operations.
	Postprocessing []json.RawMessage `json:"postprocessing"`
	// Preprocessing holds a list of configurations for
	// image preprocessing operations.
	Preprocessing []json.RawMessage `json:"preprocessing"`

	// p is a pointer to the C++ API class.
	p C.Proc
}

// NewProcessor constructs (C call) a new image processing API.
// WARNING: Delete must be called to release the memory when no longer needed.
func NewProcessor() *Processor {
	return &Processor{p: C.Proc_New()}
}

// DecodeImage decodes and stores an image from the provided buffer.
// WARNING: the image buffer MUST be kept alive until the function returns.
func (ip Processor) DecodeImage(buf []byte, readMode ReadMode) error {
	if len(buf) <= 0 {
		return errors.New("image.Processor.DecodeImage: empty image buffer")
	}
	ok := C.Proc_DecodeImage(
		ip.p,
		unsafe.Pointer(&buf[0]),
		C.int(len(buf)),
		C.int(readMode),
	)
	if !ok {
		return errors.New("image.Processor.DecodeImage: could not decode image")
	}
	return nil
}

// Delete releases C-allocated memory. Once called, ip is no longer valid.
func (ip *Processor) Delete() {
	C.Proc_Delete(ip.p)
}

// Init initializes the C-allocated API with the configuration data,
// if ip is valid.
func (ip Processor) Init() error {
	if err := ip.IsValid(); err != nil {
		return err
	}
	op := make(map[string]json.RawMessage, 1)
	helper := func(msgs []json.RawMessage) ([]unsafe.Pointer, error) {
		ptrs := make([]unsafe.Pointer, 0, len(msgs))
		for _, m := range msgs {
			if err := json.Unmarshal(m, &op); err != nil {
				return nil, err
			}
			if len(op) != 1 {
				return nil, fmt.Errorf("image.Processor.Init: too many fields %v", op)
			}
			for k, v := range op {
				f, ok := opFactoryMap[k]
				if !ok {
					return nil, fmt.Errorf("image.Processor.Init: bad operation: %v", k)
				}
				// C will manage this memory, we don't have to clean it up
				ptr, err := f(v)
				if err != nil {
					return nil, fmt.Errorf("image.Processor.Init: %w", err)
				}
				ptrs = append(ptrs, ptr)
			}
			clear(op)
		}
		return ptrs, nil
	}
	post, err := helper(ip.Postprocessing)
	if err != nil {
		return err
	}
	pre, err := helper(ip.Preprocessing)
	if err != nil {
		return err
	}
	var ppost *unsafe.Pointer
	if len(post) > 0 {
		ppost = &post[0]
	}
	var ppre *unsafe.Pointer
	if len(pre) > 0 {
		ppre = &pre[0]
	}
	ok := C.Proc_Init(
		ip.p,
		ppost,
		C.size_t(len(post)),
		ppre,
		C.size_t(len(pre)),
	)
	if !ok {
		return errors.New("image.Processor.Init: could not initialize image processing")
	}
	return nil
}

// IsValid is function used as an assertion that ip is able to be initialized.
func (ip Processor) IsValid() error {
	if ip.p == (C.Proc)(nil) {
		return errors.New("image.Processor.IsValid: nil API pointer")
	}
	return nil
}

// Postprocess runs all currently stored postprocessing operations
// on the current image.
// Note that Postprocess is usually only run after text detection/recognition
// since some postprocessing operations may depend on detection/recognition
// results.
// FIXME: this shouldn't need a Tesseract, it should take an image.Image
func (ip Processor) Postprocess(tess unsafe.Pointer) error {
	if ok := C.Proc_Postprocess(ip.p, C.Tess(tess)); !ok {
		return errors.New("image.Processor.Postprocess: could not postprocess image")
	}
	return nil
}

// Preprocess runs all currently stored preprocessing operations
// on the current image.
func (ip Processor) Preprocess() error {
	if ok := C.Proc_Preprocess(ip.p); !ok {
		return errors.New("image.Processor.Preprocess: could not preprocess image")
	}
	return nil
}

// Ptr returns a pointer to the underlying C-API.
// FIXME: nope --- remove this
func (ip Processor) Ptr() unsafe.Pointer {
	return unsafe.Pointer(ip.p)
}

// ReadImage reads and stores an image from disc.
func (ip Processor) ReadImage(filename string, readMode ReadMode) error {
	cs := C.CString(filename)
	defer C.free(unsafe.Pointer(cs))
	if ok := C.Proc_ReadImage(ip.p, cs, C.int(readMode)); !ok {
		return errors.New("image.Processor.ReadImage: could not read image")
	}
	return nil
}

// WriteAcquisitionResult writes 'r' to disc in PNG format.
func (ip Processor) ReceiveRawImage(img neutral.Image) error {
	ri := C.RawImage{
		id:    C.size_t(img.ID),
		rows:  C.int(img.Rows),
		cols:  C.int(img.Cols),
		pxTyp: C.int64_t(img.PixelType),
		buf:   img.Buffer,
		step:  C.size_t(img.Step),
	}
	if ok := C.Proc_ReceiveRawImage(ip.p, &ri); !ok {
		return errors.New("image.Processor.ReceiveRawImage: could not convert image")
	}
	return nil
}

// ShowImage renders the current image in a new window and
// waits for a key press.
//
// Deprecated: this function crashes on darwin when not called from the main
// thread.
// There are ways around this, but they are all overly complex (and ugly)
// for what we need and could potentially cause more critical issues.
// Hence, any functionality requiring a GUI, regardless of the platform,
// should be implemented through the web app.
func (ip Processor) ShowImage(title string) {
	cs := C.CString(title)
	defer C.free(unsafe.Pointer(cs))
	C.Proc_ShowImage(ip.p, cs)
}

// Write writes the currently held image to disc.
// The format of the image is determined from the filename extension,
// see OpenCV 'cv::imwrite' for supported formats.
func (ip Processor) WriteImage(filename string) error {
	cs := C.CString(filename)
	defer C.free(unsafe.Pointer(cs))
	if ok := C.Proc_WriteImage(ip.p, cs); !ok {
		return errors.New("image.Processor.WriteImage: could not write image")
	}
	return nil
}
