package neural

/*
#include <stdlib.h>
#include "neural.h"
*/
import "C"
import (
	"errors"
	"fmt"
	"os"
	"path"
	"strings"
	"unsafe"

	"github.com/Milover/beholder/internal/enumutils"
	"github.com/Milover/beholder/internal/mem"
	"github.com/Milover/beholder/internal/models"
	"github.com/Milover/beholder/internal/neural/model"
)

func init() {
	RegisterNetwork(TypeTesseract, "tesseract", func() Network { return NewTesseract() })
}

const TypeTesseract Type = 1 // tesseract OCR models

// Tesseract is a text detection and recognition [Network] using
// the [Tesseract OCR model] (engine).
//
// WARNING: Tesseract contains C-managed resources so when it is no longer
// needed, [Tesseract.Delete] must be called to release the resources and
// clean up.
//
// [Tesseract OCR model]: https://github.com/tesseract-ocr/tesseract
type Tesseract struct {
	// ConfigPaths is a list of configuration file paths.
	ConfigPaths []string `json:"config_paths"`
	// Model is the tesseract OCR  model.
	// It can be either an embedded model keyword, or a model file name.
	Model model.Model `json:"model"`
	// PageSegMode is the page segmentation mode.
	PageSegMode PSegMode `json:"page_seg_mode"`
	// Patterns is a list of Tesseract [user patterns].
	// For more info about the usage and format, see [trie.h].
	//
	// [user patterns]: https://tesseract-ocr.github.io/tessdoc/APIExample-user_patterns.html
	// [trie.h]: https://github.com/tesseract-ocr/tesseract/blob/main/src/dict/trie.h#L184
	Patterns []string `json:"patterns"`
	// Variables are runtime settable variables.
	// Here are some commonly used variables and their values:
	//
	//	"load_system_dawg":          "0"
	//	"load_freq_dawg":            "0"
	//	"classify_bln_numeric_mode": "1"
	//	"tessedit_char_whitelist":   ".,:;0123456789"
	Variables map[string]string `json:"variables"`

	p C.Tess // pointer to the C++ API class.
}

// newTesseractCPtr is a helper function which wraps the C-call which allocates
// a new Tesseract C-API and returns a pointer to it.
func newTesseractCPtr() C.Tess {
	return C.Tess_New()
}

// NewTesseract constructs (C call) a new tesseract API with sensible defaults.
// WARNING: Delete must be called to release the memory when no longer needed.
func NewTesseract() *Tesseract {
	return &Tesseract{
		PageSegMode: PSMSingleBlock,
		Variables:   map[string]string{},
		p:           newTesseractCPtr(),
	}
}

// Delete releases C-allocated memory. Once called, t is no longer valid.
func (t *Tesseract) Delete() {
	C.Tess_Delete(t.p)
}

// Clear resets C-struct internals.
// It should be called each time before running a new detect/recognize job.
// Note that t is still valid and initialized after calling Clear.
func (t Tesseract) Clear() {
	C.Tess_Clear(t.p)
}

// Inference runs the inferencing step on the supplied image. Inferencing
// includes both text detection and text recognition.
// Before calling Inference, t must be initialized by calling [Tesseract.Init].
//
// Internally stored results are cleared by the C-API when Inference is called.
func (t Tesseract) Inference(img models.Image, res *models.Result) error {
	if err := t.setImage(img); err != nil {
		return fmt.Errorf("neural.Tesseract.Inference: %w: %w", ErrInference, err)
	}
	ar := &mem.Arena{}
	defer ar.Free()

	results := (*C.ResArr)(ar.Store(
		unsafe.Pointer(C.Tess_Recognize(t.p)),
		unsafe.Pointer(C.ResArr_Delete)),
	)
	if unsafe.Pointer(results) == nil {
		return fmt.Errorf("neural.Tesseract.Inference: %w", ErrInference)
	}
	fromCRes(results, res)
	return nil
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
	// handle the model
	mfn, cleanup, err := t.Model.File()
	if err != nil {
		return err
	}
	defer cleanup() // this could fail, but we don't care

	ar := &mem.Arena{}
	defer ar.Free()

	// allocate the struct and handle the easy stuff (ints, strings...)
	in := C.TInit{
		psMode:    C.int(t.PageSegMode),
		modelPath: (*C.char)(ar.CopyStr(path.Dir(mfn))),
		model:     (*C.char)(ar.CopyStr(strings.TrimSuffix(path.Base(mfn), ".traineddata"))),
	}
	// handle configuration file names
	in.nCfgs = C.size_t(len(t.ConfigPaths))
	in.cfgs = (**C.char)(ar.CopyStrArray(t.ConfigPaths))
	// handle variables
	in.nVars = C.size_t(len(t.Variables))
	in.vars = (*C.KeyVal)(ar.Malloc(uint64(in.nVars) * uint64(unsafe.Sizeof(C.KeyVal{}))))
	varsSlice := unsafe.Slice(in.vars, uint64(in.nVars))
	iVar := 0
	for key, val := range t.Variables {
		varsSlice[iVar].key = (*C.char)(ar.CopyStr(key))
		varsSlice[iVar].value = (*C.char)(ar.CopyStr(val))
		iVar++
	}

	if ok := C.Tess_Init(t.p, &in); !ok {
		return errors.New("neural.Tesseract.Init: could not initialize tesseract")
	}
	return nil
}

// IsValid is function used as an assertion that t is able to be initialized.
func (t Tesseract) IsValid() error {
	if t.p == (C.Tess)(nil) {
		return fmt.Errorf("neural.Tesseract.IsValid: %w", ErrAPIPtr)
	}
	for _, c := range t.ConfigPaths {
		if _, err := os.Stat(c); err != nil {
			return err
		}
	}
	return nil
}

// setImage sets the image on which text detection/recognition will be run.
// It also clears the previous image and detection/recognition results.
func (t Tesseract) setImage(img models.Image) error {
	raw := toCImg(img)
	if ok := C.Tess_SetImage(t.p, &raw); !ok {
		return errors.New("neural.Tesseract.setImage: could not set image")
	}
	return nil
}

// setPatterns sets up the stuff necessary for Tesseract
// to read/load user-patterns.
// TODO: we should check that the pattern is valid, otherwise Tesseract
// will silently ignore it.
func (t *Tesseract) setPatterns() (string, error) {
	if len(t.Patterns) == 0 {
		return "", nil
	}
	f, err := os.CreateTemp("", "tesseract_patterns_")
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

// PSegMode is the page segmentation mode.
type PSegMode int

// String returns a string representation of m.
func (m PSegMode) String() string {
	s, ok := pSegModeMap[m]
	if !ok {
		return "unknown"
	}
	return s
}

// UnmarshallJSON unmarshals m from JSON.
func (m *PSegMode) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, m, invPSegModeMap)
}

// MarshallJSON marshals m into JSON.
func (m PSegMode) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(m, pSegModeMap)
}

const (
	// FIXME: yolo
	PSMOSDOnly             PSegMode = iota // orientation and script detection only
	PSMAutoOSD                             // automatic page segmentation with orientation and script detection (OSD)
	PSMAutoOnly                            // automatic page segmentation, but no OSD, or OCR
	PSMAuto                                // fully automatic page segmentation, but no OSD
	PSMSingleColumn                        // assume a single column of text of variable sizes
	PSMSingleBlockVertText                 // assume a single uniform block of vertically aligned text
	PSMSingleBlock                         // assume a single uniform block of text
	PSMSingleLine                          // treat image as a single text line
	PSMSingleWord                          // treat image as a single word
	PSMCircleWord                          // treat image as a single word in a circle
	PSMSingleChar                          // treat image as a single character
	PSMSparseText                          // find as much text as possible in no particular order
	PSMSparseTextOSD                       // sparse text with OSD
	PSMRawLine                             // treat image as a single text line, bypassing Tesseract-specific hacks
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
