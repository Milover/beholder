// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package imgproc

/*
#include <stdlib.h>
#include "imgproc.h"
*/
import "C"
import (
	"encoding/json"
	"errors"
	"fmt"
	"strings"
	"time"
	"unsafe"

	"github.com/Milover/beholder/internal/mem"
	"github.com/Milover/beholder/internal/models"
)

type ReadMode int

const (
	RMUnchanged ReadMode = iota - 1
	RMGrayscale
	RMColor
	RMAnyDepth
	RMAnyColor = 4
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
		return errors.New("imgproc.Processor.DecodeImage: empty image buffer")
	}
	ok := C.Proc_DecodeImage(
		ip.p,
		unsafe.Pointer(&buf[0]),
		C.int(len(buf)),
		C.int(readMode),
	)
	if !ok {
		return errors.New("imgproc.Processor.DecodeImage: could not decode image")
	}
	return nil
}

// Delete releases C-allocated memory. Once called, ip is no longer valid.
func (ip *Processor) Delete() {
	C.Proc_Delete(ip.p)
}

// EncodeImage encodes the currently held image (ROI) and returns the raw bytes
// of the encoding. The encoding format is determined by the format file
// extension which must include the leading period.
// See [OpenCV docs] for supported formats.
//
// TODO: use an enum instead of an extension string for setting the
// output format.
// TODO: avoid copying the encoded bytes.
//
// [OpenCV docs]: https://docs.opencv.org/4.10.0/d4/da8/group__imgcodecs.html#ga4e9883ae1f619bcbe875b7038520ea78
func (ip Processor) EncodeImage(ext string) ([]byte, error) {
	if !strings.HasPrefix(ext, ".") {
		return nil, fmt.Errorf("imgproc.Processor.EncodeImage: bad format: %q", ext)
	}
	cs := C.CString(ext)
	defer C.free(unsafe.Pointer(cs))

	var nBytes int
	b := C.Proc_EncodeImage(ip.p, cs, (*C.int)(unsafe.Pointer(&nBytes)))
	if b == (*C.uchar)(nil) {
		return nil, errors.New("imgproc.Processor.EncodeImage: could not encode image")
	}
	// the returned *C.char is managed by C, so we DON'T free/clean up
	bSlice := unsafe.Slice((*byte)(unsafe.Pointer(b)), nBytes)

	enc := make([]byte, nBytes)
	copy(enc, bSlice)
	return enc, nil
}

// GetRawImage returns the currently stored image as a [models.Image].
func (ip Processor) GetRawImage() models.Image {
	raw := C.Proc_GetRawImage(ip.p)
	return models.Image{
		ID:           uint64(raw.id),
		Timestamp:    time.Now(),
		Buffer:       unsafe.Pointer(raw.buffer),
		Rows:         int(raw.rows),
		Cols:         int(raw.cols),
		PixelType:    int64(raw.pixelType),
		Step:         uint64(raw.step),
		BitsPerPixel: uint64(raw.bitsPerPixel),
	}
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
				return nil, fmt.Errorf("imgproc.Processor.Init: too many fields %v", op)
			}
			for k, v := range op {
				f, ok := opFactoryMap[k]
				if !ok {
					return nil, fmt.Errorf("imgproc.Processor.Init: bad operation: %v", k)
				}
				// C will manage this memory, we don't have to clean it up
				ptr, err := f(v)
				if err != nil {
					return nil, fmt.Errorf("imgproc.Processor.Init: %w", err)
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
		return errors.New("imgproc.Processor.Init: could not initialize image processing")
	}
	return nil
}

// IsValid is function used as an assertion that ip is able to be initialized.
func (ip Processor) IsValid() error {
	if ip.p == (C.Proc)(nil) {
		return errors.New("imgproc.Processor.IsValid: nil API pointer")
	}
	return nil
}

// Postprocess runs all currently stored postprocessing operations
// on the current image.
// Note that Postprocess is usually only run after text detection/recognition
// since some postprocessing operations may depend on detection/recognition
// results.
func (ip Processor) Postprocess(res *models.Result) error {
	ar := &mem.Arena{}
	defer ar.Free()

	// a Result should always have associated Confidences
	// FIXME: this will break at some point
	cres := make([]C.Res, len(res.Confidences))
	for i := range res.Confidences {
		cres[i].confidence = C.double(res.Confidences[i])
		if len(res.Boxes) > i {
			cres[i].box.left = C.int(res.Boxes[i].Left)
			cres[i].box.top = C.int(res.Boxes[i].Top)
			cres[i].box.right = C.int(res.Boxes[i].Right)
			cres[i].box.bottom = C.int(res.Boxes[i].Bottom)
		}
		if len(res.Angles) > i {
			cres[i].boxRotAngle = C.double(res.Angles[i])
		}
		if len(res.Text) > i {
			cres[i].text = (*C.char)(ar.CopyStr(res.Text[i]))
		}
	}
	if ok := C.Proc_Postprocess(ip.p, &cres[0], C.size_t(len(cres))); !ok {
		return errors.New("imgproc.Processor.Postprocess: could not postprocess image")
	}
	return nil
}

// Preprocess runs all currently stored preprocessing operations
// on the current image.
func (ip Processor) Preprocess() error {
	if ok := C.Proc_Preprocess(ip.p); !ok {
		return errors.New("imgproc.Processor.Preprocess: could not preprocess image")
	}
	return nil
}

// ReadImage reads and stores an image from disc.
func (ip Processor) ReadImage(filename string, readMode ReadMode) error {
	cs := C.CString(filename)
	defer C.free(unsafe.Pointer(cs))
	if ok := C.Proc_ReadImage(ip.p, cs, C.int(readMode)); !ok {
		return errors.New("imgproc.Processor.ReadImage: could not read image")
	}
	return nil
}

// WriteAcquisitionResult writes 'r' to disc in PNG format.
func (ip Processor) ReceiveRawImage(img models.Image) error {
	ri := C.Img{
		id:           C.size_t(img.ID),
		rows:         C.int(img.Rows),
		cols:         C.int(img.Cols),
		pixelType:    C.int64_t(img.PixelType),
		buffer:       img.Buffer,
		step:         C.size_t(img.Step),
		bitsPerPixel: C.size_t(img.BitsPerPixel),
	}
	if ok := C.Proc_ReceiveRawImage(ip.p, &ri); !ok {
		return errors.New("imgproc.Processor.ReceiveRawImage: could not convert image")
	}
	return nil
}

// ResetROI resets the region of interest back to the whole image.
func (ip Processor) ResetROI() {
	C.Proc_ResetROI(ip.p)
}

// SetROI sets the region of interest to the region specified by roi.
func (ip Processor) SetROI(roi models.Rectangle) {
	r := C.Rect{
		left:   C.int(roi.Left),
		top:    C.int(roi.Top),
		right:  C.int(roi.Right),
		bottom: C.int(roi.Bottom),
	}
	C.Proc_SetROI(ip.p, &r)
}

// SetRoteedROI sets the region of interest to the region specified by roi
// rotated by ang degrees about it's center.
func (ip Processor) SetRotatedROI(roi models.Rectangle, ang float64) {
	r := C.Rect{
		left:   C.int(roi.Left),
		top:    C.int(roi.Top),
		right:  C.int(roi.Right),
		bottom: C.int(roi.Bottom),
	}
	C.Proc_SetRotatedROI(ip.p, &r, C.double(ang))
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

// ToColor converts the image to a color (BGR) image and resets
// the region interest back to the whole image.
func (ip Processor) ToColor() {
	C.Proc_ToColor(ip.p)
}

// ToGrayscale converts the image to a grayscale image and resets
// the region interest back to the whole image.
func (ip Processor) ToGrayscale() {
	C.Proc_ToGrayscale(ip.p)
}

// Write writes the currently held image to disc.
// The format of the image is determined from the filename extension.
// See [OpenCV docs] for supported formats.
//
// [OpenCV docs]: https://docs.opencv.org/4.10.0/d4/da8/group__imgcodecs.html#ga8ac397bd09e48851665edbe12aa28f25
func (ip Processor) WriteImage(filename string) error {
	cs := C.CString(filename)
	defer C.free(unsafe.Pointer(cs))
	if ok := C.Proc_WriteImage(ip.p, cs); !ok {
		return errors.New("imgproc.Processor.WriteImage: could not write image")
	}
	return nil
}
