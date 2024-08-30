package ocr

/*
#include <stdlib.h>
#include "ocr.h"
*/
import "C"
import (
	"errors"
	"fmt"
	"os"
	"path"
	"slices"
	"strings"
	"unsafe"

	"github.com/Milover/beholder/internal/enumutils"
	"github.com/Milover/beholder/internal/mem"
	"github.com/Milover/beholder/internal/neutral"
	"github.com/Milover/beholder/internal/ocr/model"
)

var (
	ErrRecognition = errors.New("could not detect/recognize text")
)

// PSegMode is the page segmentation mode.
type PSegMode int

const (
	// FIXME: yolo
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
func (t *Tesseract) Delete() {
	C.Tess_Delete(t.p)
}

// Clear resets C-struct internals.
// It should be called each time before running a new detect/recognize job.
// Note that t is still valid and initialized after calling Clear.
func (t Tesseract) Clear() {
	C.Tess_Clear(t.p)
}

// Recognize runs a new detect/recognize job on the current image.
// Before calling Recognize, t should be initialized with [Tesseract.Init], and
// have an image set with [Tesseract.SetImage].
//
// [Tesseract.Clear] should be called before each new Recognize call, however
// [Tesseract.SetImage] also clears previous results.
func (t Tesseract) Recognize(res *neutral.Result) error {
	ar := &mem.Arena{}
	defer ar.Free()

	results := (*C.ResArr)(ar.Store(
		unsafe.Pointer(C.Tess_Recognize(t.p)),
		unsafe.Pointer(C.ResArr_Delete)),
	)
	if unsafe.Pointer(results) == nil {
		return fmt.Errorf("ocr.Tesseract.Recognize: %w", ErrRecognition)
	}
	// allocate and reset if necessary
	nLines := uint64(results.count)
	if uint64(cap(res.Text)) < nLines {
		diff := int(nLines - uint64(cap(res.Text)))
		res.Text = slices.Grow(res.Text, diff)
		res.Confidences = slices.Grow(res.Confidences, diff)
		res.Boxes = slices.Grow(res.Boxes, diff)
	}
	// FIXME: we probably shouldn't do this here
	res.Text = res.Text[:0]
	res.Confidences = res.Confidences[:0]
	res.Boxes = res.Boxes[:0]
	// populate the result
	resultsSl := unsafe.Slice(results.array, nLines)
	for _, r := range resultsSl {
		res.Text = append(res.Text, C.GoString(r.text))
		res.Confidences = append(res.Confidences, float64(r.confidence))
		res.Boxes = append(res.Boxes, neutral.Rectangle{
			Left:   int64(r.box.left),
			Top:    int64(r.box.top),
			Right:  int64(r.box.right),
			Bottom: int64(r.box.bottom),
		})
	}
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
	in.cfgs = (**C.char)(ar.Malloc((uint64(in.nCfgs) * uint64(unsafe.Sizeof((*C.char)(nil))))))
	cfgsSlice := unsafe.Slice(in.cfgs, uint64(in.nCfgs))
	for i, p := range t.ConfigPaths {
		cfgsSlice[i] = (*C.char)(ar.CopyStr(p))
	}
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
	return nil
}

// SetImage sets the image on which text detection/recognition will be run.
// It also clears the previous image and detection/recognition results.
// FIXME: bytesPerPixel should be automatically determined.
func (t Tesseract) SetImage(img neutral.Image, bytesPerPixel int) error {
	raw := C.Img{
		C.size_t(img.ID),
		C.int(img.Rows),
		C.int(img.Cols),
		C.int64_t(img.PixelType),
		img.Buffer,
		C.size_t(img.Step),
		C.size_t(img.BitsPerPixel),
	}
	if ok := C.Tess_SetImage(t.p, &raw); !ok {
		return errors.New("ocr.Tesseract.SetImage: could not set image")
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
