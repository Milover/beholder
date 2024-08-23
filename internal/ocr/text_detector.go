package ocr

// TODO: both EASTTD and DBTD share most of the code apart from the
// configuration (they're also very similar to Tesseract) so we should
// be able to have one type and just register relevant functions to it.
//
// This should significantly cut down the boiler plate and make the code
// more clear and concise.
//
// We should also try to consolidate Tesseract and the Detector C-APIs,
// which should further simplify the Go implementation.

/*
#include <stdlib.h>
#include "ocr.h"
*/
import "C"
import (
	"errors"
	"fmt"
	"path"
	"slices"
	"unsafe"

	"github.com/Milover/beholder/internal/enumutils"
	"github.com/Milover/beholder/internal/mem"
	"github.com/Milover/beholder/internal/neutral"
	"github.com/Milover/beholder/internal/ocr/model"
)

var (
	ErrDetection = errors.New("could not detect text")
)

type TextDetector interface {
	// Delete releases C-allocated memory.
	// Once Delete is called, TextDetector is no longer valid.
	Delete()
	// Clear clears results held internally by the C-API.
	// TextDetector is still valid and initialized after calling Clear.
	Clear()
	// Detect performs text detection on the supplied image.
	// Before calling Detect, TextDetector must be initialized by
	// calling Init.
	//
	// Internally stored results are cleared with Clear on each Detect call.
	Detect(neutral.Image, *neutral.Result) error
	// Init initializes the C-allocated API with the configuration data,
	// if TextDetector is valid.
	Init() error
}

// TextDetectorType represents available text detector types.
type TextDetectorType int

const (
	TDTEAST TextDetectorType = iota
	TDTDB
)

var (
	textDetectorTypeMap = map[TextDetectorType]string{
		TDTDB:   "db",
		TDTEAST: "east",
	}
	invTextDetectorTypeMap = enumutils.Invert(textDetectorTypeMap)
)

func (t *TextDetectorType) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, t, invTextDetectorTypeMap)
}

func (t TextDetectorType) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(t, textDetectorTypeMap)
}

// tdFactory is a function which creates a new text detector with a default
// configuration.
type tdFactory func() TextDetector

// tdFactoryMap maps the names of image processing operations to their
// factory functions.
var tdFactoryMap = map[TextDetectorType]tdFactory{
	TDTDB:   NewDBTD,
	TDTEAST: NewEASTTD,
}

// DBTD is a handle for the DB text detector API
// and contains API configuration data.
// WARNING: DBTD holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
type DBTD struct {
	// Model is the DB model.
	// It can be either an embedded model keyword, or a model file name.
	//
	// FIXME: can't actually be a keyword at the moment.
	Model model.Model `json:"model"`

	// BinaryThreshold is the binary map threshold.
	//
	// The default value is 0.3.
	BinaryThreshold float32 `json:"binary_threshold"`
	// PolygonThreshold is the text polygon threshold.
	// The usual values are between 0.5 and 0.7.
	//
	// The default value is 0.5.
	PolygonThreshold float32 `json:"polygon_threshold"`
	// MaxCandidates is the maximum number of output results.
	//
	// The default value is 50.
	MaxCandidates int `json:"max_candidates"`
	// UnclipRatio is the unclip ratio of the detected text region, which
	// determines the output size.
	//
	// The default value is 2.0.
	UnclipRatio float64 `json:"unclip_ratio"`
	// UseHardCodedMean toggles whether to subtract a hard-coded mean value
	// from the image before inferencing.
	// If set to false, the computed mean for the current image is used.
	//
	// For more details, see OpenCV's [text spotting tutorial] documentation.
	//
	// [text spotting tutorial]: https://docs.opencv.org/4.10.0/d4/d43/tutorial_dnn_text_spotting.html
	UseHardCodedMean bool `json:"use_hard_coded_mean"`

	// p is a pointer to the C++ API class.
	p C.DB
}

// NewDBTD constructs (C call) a new DBTD API with sensible defaults.
// WARNING: Delete must be called to release the memory when no longer needed.
//
// FIXME: would be nicer if we could return *DBTD, instead of the interface.
func NewDBTD() TextDetector {
	return &DBTD{
		BinaryThreshold:  0.3,
		PolygonThreshold: 0.5,
		MaxCandidates:    50,
		UnclipRatio:      2.0,
		p:                C.DB_New(),
	}
}

// Delete releases C-allocated memory. Once called, d is no longer valid.
func (d *DBTD) Delete() {
	C.Det_Delete(C.Det(unsafe.Pointer(d.p)))
}

// Clear clears results held internally by the C-API.
// Note that d is still valid and initialized after calling Clear.
func (d DBTD) Clear() {
	C.Det_Clear(C.Det(unsafe.Pointer(d.p)))
}

// Detect performs text detection on the supplied image.
// Before calling Detect, d must be initialized by calling [DBTD.Init].
//
// Internally stored results are cleared with [DBTD.Clear] on each Detect call.
func (d DBTD) Detect(img neutral.Image, res *neutral.Result) error {
	d.Clear()

	fn := func(raw *C.RawImage) unsafe.Pointer {
		return unsafe.Pointer(C.Det_Detect(C.Det(unsafe.Pointer(d.p)), raw))
	}
	if err := detect(fn, img, res); err != nil {
		return fmt.Errorf("ocr.DBTD.Detect: %w", err)
	}
	return nil
}

// Init initializes the C-allocated API with the configuration data,
// if d is valid.
func (d DBTD) Init() error {
	if err := d.IsValid(); err != nil {
		return err
	}
	ar := &mem.Arena{}
	defer ar.Free()

	// allocate the struct and handle the easy stuff (ints, strings...)
	in := C.DBInit{
		binary:    C.float(d.BinaryThreshold),
		polygon:   C.float(d.PolygonThreshold),
		maxCand:   C.int(d.MaxCandidates),
		unclip:    C.double(d.UnclipRatio),
		useHCMean: C.bool(d.UseHardCodedMean),
	}
	// handle the model
	mfn, remover, err := d.Model.File()
	if err != nil {
		return err
	}
	defer remover() // this could fail, but we don't care
	in.modelPath = (*C.char)(ar.CopyStr(path.Dir(mfn)))
	in.model = (*C.char)(ar.CopyStr(path.Base(mfn)))

	if ok := C.DB_Init(d.p, &in); !ok {
		return errors.New("ocr.DBTD.Init: could not initialize DB text detector")
	}
	return nil
}

// IsValid is function used as an assertion that d is able to be initialized.
func (d DBTD) IsValid() error {
	if d.p == C.DB(nil) {
		return errors.New("ocr.DBTD.IsValid: nil API pointer")
	}
	if d.BinaryThreshold < 0.0 || d.BinaryThreshold > 1.0 {
		return errors.New("ocr.DBTD.IsValid: bad binary threshold")
	}
	if d.PolygonThreshold < 0.0 || d.PolygonThreshold > 1.0 {
		return errors.New("ocr.DBTD.IsValid: bad polygon threshold")
	}
	if d.MaxCandidates <= 0 {
		return errors.New("ocr.DBTD.IsValid: bad max candidates")
	}
	if d.UnclipRatio <= 0.0 {
		return errors.New("ocr.DBTD.IsValid: bad unclip ratio")
	}
	if err := d.Model.IsValid(); err != nil {
		return err
	}
	return nil
}

// EASTTD is a handle for the EAST text detector API
// and contains API configuration data.
// WARNING: EASTTD holds a pointer to C-allocated memory,
// so when it is no longer needed, Delete must be called to release
// the memory and clean up.
type EASTTD struct {
	// Model is the EAST model.
	// It can be either an embedded model keyword, or a model file name.
	//
	// FIXME: can't actually be a keyword at the moment.
	Model model.Model `json:"model"`

	// ConfidenceThreshold is the minimum confidence needed to accept
	// a detected text box.
	ConfidenceThreshold float32 `json:"confidence_threshold"`
	// NMSThreshold is the non-maximum suppression threshold.
	NMSThreshold float32 `json:"nms_threshold"`
	// UseHardCodedMean toggles whether to subtract a hard-coded mean value
	// from the image before inferencing.
	// If set to false, the computed mean for the current image is used.
	//
	// For more details, see OpenCV's [text spotting tutorial] documentation.
	//
	// [text spotting tutorial]: https://docs.opencv.org/4.10.0/d4/d43/tutorial_dnn_text_spotting.html
	UseHardCodedMean bool `json:"use_hard_coded_mean"`

	// p is a pointer to the C++ API class.
	p C.EAST
}

// NewEASTTD constructs (C call) a new EASTTD API with sensible defaults.
// WARNING: Delete must be called to release the memory when no longer needed.
//
// FIXME: would be nicer if we could return *EASTTD, instead of the interface.
func NewEASTTD() TextDetector {
	return &EASTTD{
		ConfidenceThreshold: 0.5,
		NMSThreshold:        0.0,
		p:                   C.EAST_New(),
	}
}

// Delete releases C-allocated memory. Once called, d is no longer valid.
func (d *EASTTD) Delete() {
	C.Det_Delete(C.Det(unsafe.Pointer(d.p)))
}

// Clear clears results held internally by the C-API.
// Note that d is still valid and initialized after calling Clear.
func (d EASTTD) Clear() {
	C.Det_Clear(C.Det(unsafe.Pointer(d.p)))
}

// Detect performs text detection on the supplied image.
// Before calling Detect, d must be initialized by calling [EASTTD.Init].
//
// Internally stored results are cleared with [EASTTD.Clear] on each Detect call.
func (d EASTTD) Detect(img neutral.Image, res *neutral.Result) error {
	d.Clear()

	fn := func(raw *C.RawImage) unsafe.Pointer {
		return unsafe.Pointer(C.Det_Detect(C.Det(unsafe.Pointer(d.p)), raw))
	}
	if err := detect(fn, img, res); err != nil {
		return fmt.Errorf("ocr.EASTTD.Detect: %w", err)
	}
	return nil
}

// Init initializes the C-allocated API with the configuration data,
// if d is valid.
func (d EASTTD) Init() error {
	if err := d.IsValid(); err != nil {
		return err
	}
	ar := &mem.Arena{}
	defer ar.Free()

	// allocate the struct and handle the easy stuff (ints, strings...)
	in := C.EASTInit{
		conf:      C.float(d.ConfidenceThreshold),
		nms:       C.float(d.NMSThreshold),
		useHCMean: C.bool(d.UseHardCodedMean),
	}
	// handle the model
	mfn, remover, err := d.Model.File()
	if err != nil {
		return err
	}
	defer remover() // this could fail, but we don't care
	in.modelPath = (*C.char)(ar.CopyStr(path.Dir(mfn)))
	in.model = (*C.char)(ar.CopyStr(path.Base(mfn)))

	if ok := C.EAST_Init(d.p, &in); !ok {
		return errors.New("ocr.EASTTD.Init: could not initialize EAST text detector")
	}
	return nil
}

// IsValid is function used as an assertion that d is able to be initialized.
func (d EASTTD) IsValid() error {
	if d.p == C.EAST(nil) {
		return errors.New("ocr.EASTTD.IsValid: nil API pointer")
	}
	if d.ConfidenceThreshold < 0.0 || d.ConfidenceThreshold > 1.0 {
		return errors.New("ocr.EASTTD.IsValid: bad confidence threshold")
	}
	if d.NMSThreshold < 0.0 || d.NMSThreshold > 1.0 {
		return errors.New("ocr.EASTTD.IsValid: bad non-maximum suppression threshold")
	}
	if err := d.Model.IsValid(); err != nil {
		return err
	}
	return nil
}

// detectFn is a helper type which wraps the detector-specific C-API call.
type detectFn func(*C.RawImage) unsafe.Pointer

// detect is a helper function which sets up and runs text detection on img,
// and transfers the results into res.
func detect(fn detectFn, img neutral.Image, res *neutral.Result) error {
	ar := &mem.Arena{}
	defer ar.Free()

	raw := C.RawImage{
		C.size_t(img.ID),
		C.int(img.Rows),
		C.int(img.Cols),
		C.int64_t(img.PixelType),
		img.Buffer,
		C.size_t(img.Step),
	}
	results := (*C.ResArr)(ar.Store(fn(&raw), unsafe.Pointer(C.ResArr_Delete)))
	if unsafe.Pointer(results) == nil {
		return ErrDetection
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
		res.Confidences = append(res.Confidences, float64(r.conf))
		res.Boxes = append(res.Boxes, neutral.Rectangle{
			Left:   int64(r.box.left),
			Top:    int64(r.box.top),
			Right:  int64(r.box.right),
			Bottom: int64(r.box.bottom),
		})
	}
	return nil
}
