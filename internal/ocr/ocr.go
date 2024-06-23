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
	"io"
	"os"
	"path"
	"strings"
	"time"
	"unsafe"

	"github.com/Milover/ocr/internal/enumutils"
	"github.com/Milover/ocr/internal/ocr/model"
	"github.com/Milover/ocr/internal/stopwatch"
)

// Result holds the OCR pipeline results and statistics.
type Result struct {
	// Text is the text recognized by the OCR.
	Text string
	// TimeStamp is the time at which the OCR pipeline started.
	TimeStamp time.Time
	// RunDuration is the time elapsed from start to finish of the OCR pipeline.
	RunDuration      time.Duration
	ReadDuration     time.Duration
	DecodeDuration   time.Duration
	PreprocDuration  time.Duration
	OCRDuration      time.Duration
	PostprocDuration time.Duration
}

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

// Run is a function that runs the OCR pipeline for a single image: reading,
// preprocessing, recognition and postprocessing.
func (o OCR) Run(r io.Reader) (Result, error) {
	sw := stopwatch.New()
	res := Result{TimeStamp: sw.Start}

	buf, err := io.ReadAll(r)
	if err != nil {
		return res, err
	}
	res.ReadDuration = sw.Lap()

	if err = o.P.DecodeImage(buf, ImreadGrayscale); err != nil {
		return res, err
	}
	res.DecodeDuration = sw.Lap()

	if err := o.P.Preprocess(); err != nil {
		return res, err
	}
	o.T.SetImage(*o.P, 1)
	res.PreprocDuration = sw.Lap()

	if res.Text, err = o.T.DetectAndRecognize(); err != nil {
		return res, err
	}
	res.OCRDuration = sw.Lap()

	if err = o.P.Postprocess(*o.T); err != nil {
		return res, err
	}
	res.PostprocDuration = sw.Lap()

	res.RunDuration = sw.Total()
	return res, nil
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

// PSegMode represents available page segmentation modes.
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

var (
	pSegModeMap = map[PSegMode]string{
		PSMOSDOnly:             "osd_only",
		PSMAutoOSD:             "auto_osd",
		PSMAutoOnly:            "auto_only",
		PSMAuto:                "auto",
		PSMSingleColumn:        "single_column",
		PSMSingleBlockVertText: "single_block_vert_text",
		PSMSingleBlock:         "single_block",
		PSMSingleLine:          "single_line",
		PSMSingleWord:          "single_word",
		PSMCircleWord:          "circle_word",
		PSMSingleChar:          "single_char",
		PSMSparseText:          "sparse_text",
		PSMSparseTextOSD:       "sparse_text_osd",
		PSMRawLine:             "raw_line",
	}
	invPSegModeMap = enumutils.Invert(pSegModeMap)
)

func (m *PSegMode) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, m, invPSegModeMap)
}

func (m PSegMode) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(m, pSegModeMap)
}

// Tesseract is a handle for the tesseract API
// and contains API configuration data.
// WARNING: Tesseract holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
type Tesseract struct {
	// ConfigurationPaths is a list of configuration file paths.
	ConfigPaths []string `json:"config_paths"`
	// Model is the tesseract OCR  model.
	// It can be either an embedded model keyword, or a model file name.
	Model model.Model `json:"model"`
	// PageSegMode is the page segmentation mode.
	PageSegMode PSegMode `json:"page_seg_mode"`
	// Patterns is a list of Tesseract user-patterns.
	// For more info about the usage and format, see:
	// https://github.com/tesseract-ocr/tesseract/blob/main/src/dict/trie.h#L184
	Patterns []string `json:"patterns"`
	// Variables are runtime settable variables.
	// Here are some commonly used variables and their values:
	//
	//	"load_system_dawg":          "0"
	//	"load_freq_dawg":            "0"
	//	"classify_bln_numeric_mode": "1"
	//	"tessedit_char_whitelist":   ".,:;0123456789"
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
	if patternsFile, err := t.setPatterns(); err != nil {
		return err
	} else {
		defer os.Remove(patternsFile) // this could fail, but we don't care
	}

	// allocate the struct and handle the easy stuff (ints, strings...)
	// NOTE: C.malloc guarantees never to return nil, no need to check
	in := (*C.TInit)(C.malloc(C.sizeof_TInit))
	defer C.free(unsafe.Pointer(in))

	// set the page segmentation mode
	in.psMode = C.int(t.PageSegMode)

	// handle the model
	mfn, remover, err := t.Model.File()
	if err != nil {
		return err
	}
	defer remover() // this could fail, but we don't care
	in.modelPath = C.CString(path.Dir(mfn))
	defer C.free(unsafe.Pointer(in.modelPath))
	in.model = C.CString(strings.TrimSuffix(path.Base(mfn), ".traineddata"))
	defer C.free(unsafe.Pointer(in.model))

	// handle configuration file names
	chPtrSize := unsafe.Sizeof((*C.char)(nil))
	in.nCfgs = C.size_t(len(t.ConfigPaths))
	in.cfgs = (**C.char)(C.malloc(in.nCfgs * C.size_t(chPtrSize)))
	defer C.free(unsafe.Pointer(in.cfgs))
	cfgsSlice := unsafe.Slice(in.cfgs, int(in.nCfgs))
	for i, p := range t.ConfigPaths {
		cfgsSlice[i] = C.CString(p)
		defer C.free(unsafe.Pointer(cfgsSlice[i]))
	}
	// handle variables
	kvSize := unsafe.Sizeof(C.KeyVal{})
	in.nVars = C.size_t(len(t.Variables))
	in.vars = (*C.KeyVal)(C.malloc(in.nVars * C.size_t(kvSize)))
	defer C.free(unsafe.Pointer(in.vars))
	varsSlice := unsafe.Slice(in.vars, int(in.nVars))
	iVar := 0
	for key, val := range t.Variables {
		varsSlice[iVar].key = C.CString(key)
		defer C.free(unsafe.Pointer(varsSlice[iVar].key))
		varsSlice[iVar].value = C.CString(val)
		defer C.free(unsafe.Pointer(varsSlice[iVar].value))
		iVar++
	}

	if ok := C.Tess_Init(t.p, in); !ok {
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
	if err := t.Model.IsValid(); err != nil {
		return err
	}
	return nil
}

// SetImage sets the image on which text detection/recognition will be run.
// It also clears the previous image and detection/recognition results.
// FIXME: bytesPerPixel should be automatically determined.
func (t Tesseract) SetImage(ip ImageProcessor, bytesPerPixel int) {
	C.Tess_SetImage(t.p, ip.p, C.int(bytesPerPixel))
}

// setPatterns sets up the stuff necessary for Tesseract
// to read/load user-patterns.
// TODO: we should check that the pattern is valid, otherwise Tesseract
// will silently ignore it.
func (t *Tesseract) setPatterns() (string, error) {
	if len(t.Patterns) == 0 {
		return "", nil
	}
	f, err := os.CreateTemp("", "ocr_patterns_")
	if err != nil {
		return "", err
	}
	defer f.Close()

	var b strings.Builder
	for i, p := range t.Patterns {
		b.WriteString(p)
		if i != len(t.Patterns)-1 {
			b.WriteRune('\n')
		}
	}
	if _, err := f.WriteString(b.String()); err != nil {
		return f.Name(), err
	}
	t.Variables["user_patterns_file"] = f.Name()

	return f.Name(), nil
}
