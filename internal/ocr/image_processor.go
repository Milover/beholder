package ocr

/*
#include <stdlib.h>
#include "ocr.h"
*/
import "C"
import (
	"encoding/json"
	"errors"
	"fmt"
	"unsafe"
)

type ImreadMode int

const (
	ImreadUnchanged ImreadMode = iota - 1
	ImreadGrayscale
	ImreadAnyDepth
	ImreadAnyColor = iota
)

// ImageProcessor is a handle for the image processing API
// and contains API configuration data.
// WARNING: ImageProcessor holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
// TODO: missing a way to reset the image/read an image from a []byte.
type ImageProcessor struct {
	// Postprocessing holds a list of configurations for
	// image postprocessing operations.
	Postprocessing []json.RawMessage `json:"postprocessing"`
	// Preprocessing holds a list of configurations for
	// image preprocessing operations.
	Preprocessing []json.RawMessage `json:"preprocessing"`

	// p is a pointer to the C++ API class.
	p C.Proc
}

// NewImageProcessor constructs (C call) a new image processing API.
// WARNING: Delete must be called to release the memory when no longer needed.
func NewImageProcessor() *ImageProcessor {
	return &ImageProcessor{p: C.Proc_New()}
}

// DecodeImage decodes and stores an image from the provided buffer.
// WARNING: the image buffer MUST be kept alive until the function returns.
func (ip ImageProcessor) DecodeImage(buf []byte, readMode ImreadMode) error {
	if len(buf) <= 0 {
		return errors.New("ocr.ImageProcessor.DecodeImage: empty image buffer")
	}
	ok := C.Proc_DecodeImage(
		ip.p,
		unsafe.Pointer(&buf[0]),
		C.int(len(buf)),
		C.int(readMode),
	)
	if !ok {
		return errors.New("ocr.ImageProcessor.DecodeImage: could not decode image")
	}
	return nil
}

// Delete releases C-allocated memory. Once called, ip is no longer valid.
func (ip *ImageProcessor) Delete() {
	C.Proc_Delete(ip.p)
}

// Init initializes the C-allocated API with the configuration data,
// if ip is valid.
func (ip ImageProcessor) Init() error {
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
				return nil, fmt.Errorf("ocr.ImageProcessor.Init: too many fields %v", op)
			}
			for k, v := range op {
				f, ok := opFactoryMap[k]
				if !ok {
					return nil, fmt.Errorf("ocr.ImageProcessor.Init: bad operation: %v", k)
				}
				// C will manage this memory, we don't have to clean it up
				ptr, err := f(v)
				if err != nil {
					return nil, fmt.Errorf("ocr.ImageProcessor.Init: %w", err)
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
		return errors.New("ocr.ImageProcessor.Init: could not initialize image processing")
	}
	return nil
}

// IsValid is function used as an assertion that ip is able to be initialized.
func (ip ImageProcessor) IsValid() error {
	if ip.p == (C.Proc)(nil) {
		return errors.New("ocr.ImageProcessor.IsValid: nil API pointer")
	}
	return nil
}

// Postprocess runs all currently stored postprocessing operations
// on the current image.
// Note that Postprocess is usually only run after text detection/recognition
// since some postprocessing operations may depend on detection/recognition
// results.
func (ip ImageProcessor) Postprocess(t Tesseract) error {
	if ok := C.Proc_Postprocess(ip.p, t.p); !ok {
		return errors.New("ocr.ImageProcessor.Postprocess: could not postprocess image")
	}
	return nil
}

// Preprocess runs all currently stored preprocessing operations
// on the current image.
func (ip ImageProcessor) Preprocess() error {
	if ok := C.Proc_Preprocess(ip.p); !ok {
		return errors.New("ocr.ImageProcessor.Preprocess: could not preprocess image")
	}
	return nil
}

// ReadImage reads and stores an image from disc.
func (ip ImageProcessor) ReadImage(filename string, readMode ImreadMode) error {
	cs := C.CString(filename)
	defer C.free(unsafe.Pointer(cs))
	if ok := C.Proc_ReadImage(ip.p, cs, C.int(readMode)); !ok {
		return errors.New("ocr.ImageProcessor.ReadImage: could not read image")
	}
	return nil
}

// CopyBayerRGGB8 takes raw bytes of a Bayer RGGB 8-bit encoded image,
// and copies it to local storage.
// The image is converted into a 3-channel 8-bit BGR color space.
// TODO: generalize so that we can receive any pixel format.
func (ip ImageProcessor) CopyBayerRGGB8(
	buf unsafe.Pointer,
	rows int64,
	cols int64,
	step uint64,
) error {
	ok := C.Proc_CopyBayerRGGB8(ip.p, C.int(rows), C.int(cols), buf, C.size_t(step))
	if !ok {
		return errors.New("ocr.ImageProcessor.ReceiveImage: could not receive image")
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
func (ip ImageProcessor) ShowImage(title string) {
	cs := C.CString(title)
	defer C.free(unsafe.Pointer(cs))
	C.Proc_ShowImage(ip.p, cs)
}

// Write writes the currently held image to disc.
// The format of the image is determined from the filename extension,
// see OpenCV 'cv::imwrite' for supported formats.
func (ip ImageProcessor) WriteImage(filename string) error {
	cs := C.CString(filename)
	defer C.free(unsafe.Pointer(cs))
	if ok := C.Proc_WriteImage(ip.p, cs); !ok {
		return errors.New("ocr.ImageProcessor.WriteImage: could not write image")
	}
	return nil
}
