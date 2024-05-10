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
	"strings"
	"unsafe"
)

// OCR is a container for Tesseract and ImageProcessor APIs.
// Since both are usually required at the same time, OCR simplifies
// construction, initialization and cleanup of the two APIs.
//
// NOTE: think whether we should restrict access to the APIs altogether,
// and just use OCR as the API interface, since Tesseract and ImageProcessor
// are almost exclusively used in tandem, for example: when reading
// a new image, Tesseract should be Clear()-ed; when preprocessing and image,
// Tesseract should SetImage(); postprocessing an image should only happen
// after running DetectAndRecognize().
// So while this would be somewhat more restrictive, it would probably make
// using the two less error prone.
type OCR struct {
	T *Tesseract      `json:"ocr"`
	P *ImageProcessor `json:"image_processing"`
}

// NewOCR constructs a new OCR.
// WARNING: since OCR's fields allocate C-memory,
// Delete must be called to release memory when no longer needed.
func NewOCR() *OCR {
	return &OCR{
		T: NewTesseract(),
		P: NewImageProcessor(),
	}
}

// Delete releases C-allocated memory by calling each field's Delete method.
// Once called, the contents (fields) of o are no longer valid.
func (o OCR) Delete() {
	o.T.Delete()
	o.P.Delete()
}

// Init initializes C-allocated APIs with configuration data by calling
// each of o's fields Init method.
func (o OCR) Init() error {
	if err := o.T.Init(); err != nil {
		return err
	}
	if err := o.P.Init(); err != nil {
		return err
	}
	return nil
}

// ImageProcessor is a handle for the image processing API
// and contains API configuration data.
// WARNING: ImageProcessor holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
// FIXME: missing a way to reset the image/read an image from a []byte.
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

// Delete releases C-allocated memory. Once called, ip is no longer valid.
func (ip ImageProcessor) Delete() {
	C.Proc_Delete(ip.p)
}

// Init initializes the C-allocated API with the configuration data.
func (ip ImageProcessor) Init() error {
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
	ok := C.Proc_Init(ip.p, &post[0], C.size_t(len(post)), &pre[0], C.size_t(len(pre)))
	if !ok {
		return errors.New("ocr.ImageProcessor.Init: could not initialize image processing")
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
func (ip ImageProcessor) ReadImage(filename string, flags int) error {
	cs := C.CString(filename)
	defer C.free(unsafe.Pointer(cs))
	if ok := C.Proc_ReadImage(ip.p, cs, C.int(flags)); !ok {
		return errors.New("ocr.ImageProcessor.ReadImage: could not read image")
	}
	return nil
}

// ShowImage renders the current image in a new window.
func (ip ImageProcessor) ShowImage(title string) {
	cs := C.CString(title)
	defer C.free(unsafe.Pointer(cs))
	C.Proc_ShowImage(ip.p, cs)
}

// Tesseract is a handle for the tesseract API
// and contains API configuration data.
// WARNING: Tesseract holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
type Tesseract struct {
	// ConfigurationPaths is a list of configuration file paths.
	ConfigPaths []string `json:"config_paths"`
	// ModelDirPath is the path to the directory containing
	// the model (trained data) file.
	ModelDirPath string `json:"model_dir_path"`
	// Model is the name of the model (trained data) file.
	Model string `json:"model"`

	// p is a pointer to the C++ API class.
	p C.Tess
}

// NewTesseract constructs (C call) a new tesseract API.
// WARNING: Delete must be called to release the memory when no longer needed.
func NewTesseract() *Tesseract {
	return &Tesseract{p: C.Tess_New()}
}

// Delete releases C-allocated memory. Once called, t is no longer valid.
func (t Tesseract) Delete() {
	C.Tess_Delete(t.p)
}

// Clear resets C-struct internals.
// It should be called each time before running a new detect/recognize job.
// Note that t is still valid and initialized after calling Clear.
func (t Tesseract) Clear() {
	C.Tess_Clear(t.p)
}

// DetectAndRecognize runs a new detect/recognize job on the current image.
// Note that before calling DetectAndRecognize, t should be initialized,
// via the Init function, and have an image set via SetImage.
// Note that Clear should be called before each new DetectAndRecognize call.
func (t Tesseract) DetectAndRecognize() (string, error) {
	ch := (*C.char)(C.Tess_DetectAndRecognize(t.p))
	defer C.free(unsafe.Pointer(ch))
	str := strings.TrimSpace(C.GoString(ch))
	if len(str) == 0 {
		return str, errors.New("ocr.Tesseract.DetectAndRecognize: could not detect/recognize text")
	}
	return str, nil
}

// Init initializes the C-allocated API with the configuration data.
func (t Tesseract) Init() error {
	cp := make([]*C.char, len(t.ConfigPaths))
	for i, p := range t.ConfigPaths {
		cp[i] = C.CString(p)
		defer C.free(unsafe.Pointer(cp[i]))
	}
	mdp := C.CString(t.ModelDirPath)
	defer C.free(unsafe.Pointer(mdp))
	m := C.CString(t.Model)
	defer C.free(unsafe.Pointer(m))
	if ok := C.Tess_Init(t.p, &cp[0], C.size_t(len(t.ConfigPaths)), mdp, m); !ok {
		return errors.New("ocr.Tesseract.Init: could not initialize tesseract")
	}
	return nil
}

// SetImage sets the image on which text detection/recognition will be run.
// FIXME: bytextPerPixel should be automatically determined.
func (t Tesseract) SetImage(ip ImageProcessor, bytesPerPixel int) {
	C.Tess_SetImage(t.p, ip.p, C.int(bytesPerPixel))
}
