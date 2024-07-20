package ocr

/*
#include <stdlib.h>
#include "ocr.h"
*/
import "C"
import (
	"errors"
	"os"
	"path"
	"strings"
	"unsafe"

	"github.com/Milover/beholder/internal/enumutils"
	"github.com/Milover/beholder/internal/ocr/model"
)

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
