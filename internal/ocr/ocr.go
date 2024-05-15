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
	"os"
	"path"
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
// a new image, Tesseract should be Clear()-ed; when preprocessing an image,
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

// Run is a function that runs the OCR pipeline (preprocessing, recognition
// and postprocessing) on the provided image.
// WARNING: the image buffer MUST be kept alive until the function returns.
func (o OCR) Run(buf []byte) (string, error) {
	if err := o.P.DecodeImage(buf, ImreadGrayscale); err != nil {
		return "", err
	}

	if err := o.P.Preprocess(); err != nil {
		return "", err
	}
	o.T.SetImage(*o.P, 1)

	text, err := o.T.DetectAndRecognize()
	if err != nil {
		return text, err
	}
	if err = o.P.Postprocess(*o.T); err != nil {
		return text, err
	}
	return text, nil
}

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
func (ip ImageProcessor) Delete() {
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

// ShowImage renders the current image in a new window.
func (ip ImageProcessor) ShowImage(title string) {
	cs := C.CString(title)
	defer C.free(unsafe.Pointer(cs))
	C.Proc_ShowImage(ip.p, cs)
}

// threshType represents the available types of threshold operations.
type PSegMode int

const (
	PSMOSDOnly PSegMode = iota
	PSMAutoOSD
	PSMAutoOnly
	PSMAuto
	PSMSingleColumn
	PSMSingleBlockVertText
	PSMSingleBlock
	PSMSingleLine
	PSMSingleWord
	PSMCircleWord
	PSMSingleChar
	PSMSparseText
	PSMSparseTextOSD
	PSMRawLine
)

func (m *PSegMode) UnmarshalJSON(data []byte) error {
	var s string
	if err := json.Unmarshal(data, &s); err != nil {
		return err
	}
	switch s {
	default:
		return fmt.Errorf("bad page seg mode: %q", s)
	case "psm_osd_only":
		*m = PSMOSDOnly
	case "psm_auto_osd":
		*m = PSMAutoOSD
	case "psm_auto_only":
		*m = PSMAutoOnly
	case "psm_auto":
		*m = PSMAuto
	case "psm_single_column":
		*m = PSMSingleColumn
	case "psm_single_block_vert_text":
		*m = PSMSingleBlockVertText
	case "psm_single_block":
		*m = PSMSingleBlock
	case "psm_single_line":
		*m = PSMSingleLine
	case "psm_single_word":
		*m = PSMSingleWord
	case "psm_circle_word":
		*m = PSMCircleWord
	case "psm_single_char":
		*m = PSMSingleChar
	case "psm_sparse_text":
		*m = PSMSparseText
	case "psm_sparse_text_osd":
		*m = PSMSparseTextOSD
	case "psm_raw_line":
		*m = PSMRawLine
	}
	return nil
}

func (m PSegMode) MarshalJSON() ([]byte, error) {
	switch m {
	case PSMOSDOnly:
		return json.Marshal("psm_osd_only")
	case PSMAutoOSD:
		return json.Marshal("psm_auto_osd")
	case PSMAutoOnly:
		return json.Marshal("psm_auto_only")
	case PSMAuto:
		return json.Marshal("psm_auto")
	case PSMSingleColumn:
		return json.Marshal("psm_single_column")
	case PSMSingleBlockVertText:
		return json.Marshal("psm_single_block_vert_text")
	case PSMSingleBlock:
		return json.Marshal("psm_single_block")
	case PSMSingleLine:
		return json.Marshal("psm_single_line")
	case PSMSingleWord:
		return json.Marshal("psm_single_word")
	case PSMCircleWord:
		return json.Marshal("psm_circle_word")
	case PSMSingleChar:
		return json.Marshal("psm_single_char")
	case PSMSparseText:
		return json.Marshal("psm_sparse_text")
	case PSMSparseTextOSD:
		return json.Marshal("psm_sparse_text_osd")
	case PSMRawLine:
		return json.Marshal("psm_raw_line")
	}
	return nil, fmt.Errorf("bad page seg mode: %d", m)
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
	// PageSegMode is the page segmentation mode
	PageSegMode PSegMode `json:"page_seg_mode"`
	// Variables are runtime settable variables
	Variables map[string]string `json:"variables"`

	// p is a pointer to the C++ API class.
	p C.Tess
}

// NewTesseract constructs (C call) a new tesseract API with sensible defaults.
// WARNING: Delete must be called to release the memory when no longer needed.
func NewTesseract() *Tesseract {
	return &Tesseract{
		PageSegMode: PSMSingleBlock,
		Variables: map[string]string{
			"load_system_dawg": "0",
			"load_freq_dawg":   "0",
		},
		p: C.Tess_New(),
	}
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

// Init initializes the C-allocated API with the configuration data,
// if t is valid.
func (t Tesseract) Init() error {
	if err := t.IsValid(); err != nil {
		return err
	}
	// allocate the struct and handle the easy stuff (ints, strings...)
	in := (*C.TInit)(C.malloc(C.sizeof_TInit))
	defer C.free(unsafe.Pointer(in))
	in.modelPath = C.CString(t.ModelDirPath)
	defer C.free(unsafe.Pointer(in.modelPath))
	in.model = C.CString(t.Model)
	defer C.free(unsafe.Pointer(in.model))
	//in.psMode = C.int(t.PageSegMode)

	chPtrSize := unsafe.Sizeof((*C.char)(nil))
	// handle configuration file names
	in.nCfgs = C.size_t(len(t.ConfigPaths))
	in.cfgs = (**C.char)(C.malloc(C.size_t(in.nCfgs) * C.size_t(chPtrSize)))
	defer C.free(unsafe.Pointer(in.cfgs))
	if in.cfgs == nil {
		return errors.New("ocr.Tesseract.Init: could not allocate configs")
	}
	cfgsSlice := unsafe.Slice(in.cfgs, int(in.nCfgs))
	for i, p := range t.ConfigPaths {
		cfgsSlice[i] = C.CString(p)
		defer C.free(unsafe.Pointer(cfgsSlice[i]))
	}
	// TODO: handle page seg mode and variables

	ok := C.Tess_Init(t.p, in)
	if !ok {
		return errors.New("ocr.Tesseract.Init: could not initialize tesseract")
	}
	return nil
}

// IsValid is function used as an assertion that t is able to be initialized.
func (t Tesseract) IsValid() error {
	if t.p == (C.Tess)(nil) {
		return errors.New("ocr.Tesseract.IsValid: nil API pointer")
	}
	for _, c := range t.ConfigPaths {
		if _, err := os.Stat(c); err != nil {
			return err
		}
	}
	if _, err := os.Stat(t.ModelDirPath); err != nil {
		return err
	}
	if _, err := os.Stat(path.Join(t.ModelDirPath, t.Model+".traineddata")); err != nil {
		return err
	}
	return nil
}

// SetImage sets the image on which text detection/recognition will be run.
// It also clears the previous image and detection/recognition results.
// FIXME: bytextPerPixel should be automatically determined.
func (t Tesseract) SetImage(ip ImageProcessor, bytesPerPixel int) {
	C.Tess_SetImage(t.p, ip.p, C.int(bytesPerPixel))
}
