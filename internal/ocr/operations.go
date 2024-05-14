package ocr

/*
#include <stdlib.h>
#include "operations.h"
*/
import "C"
import (
	"encoding/json"
	"errors"
	"fmt"
	"unsafe"
)

// opFactory is a function which creates a new image processing operation
// using a mix of default values and runtime supplied data.
// WARNING: the operation is C-allocated and will be managed by C,
// hence C.free should NOT be called on the returned pointer.
type opFactory func(json.RawMessage) (unsafe.Pointer, error)

// opFactoryMap maps the names of image processing operations to their
// factory functions.
var opFactoryMap = map[string]opFactory{
	"auto_crop":                     NewAutoCrop,
	"crop":                          NewCrop,
	"draw_text_boxes":               NewDrawTextBoxes,
	"gaussian_blur":                 NewGaussianBlur,
	"invert":                        NewInvert,
	"median_blur":                   NewMedianBlur,
	"morphology":                    NewMorphology,
	"normalize_brightness_contrast": NewNormBC,
	"resize":                        NewResize,
	"rotate":                        NewRotate,
	"threshold":                     NewThreshold,
}

// autoCrop represents an automatic image cropping operation.
// It currently uses morphologic gradients to detect text boxes and
// then crops the image.
type autoCrop struct {
	KernelSize int     `json:"kernel_size"`
	TextWidth  float32 `json:"text_width"`
	TextHeight float32 `json:"text_height"`
	Padding    float32 `json:"padding"`
}

// NewAutoCrop creates an automaticcropping operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewAutoCrop(m json.RawMessage) (unsafe.Pointer, error) {
	op := autoCrop{
		KernelSize: 50,
		TextWidth:  50,
		TextHeight: 50,
		Padding:    10,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.KernelSize <= 0 {
		return nil, errors.New("ocr.NewAutoCrop: bad kernel size")
	}
	if op.Padding <= 0 {
		return nil, errors.New("ocr.NewAutoCrop: bad padding")
	}
	// TextWidth and TextHeight accept all values
	return unsafe.Pointer(C.AuCrp_New(
		C.int(op.KernelSize),
		C.float(op.TextWidth),
		C.float(op.TextHeight),
		C.float(op.Padding),
	)), nil
}

// crop represents an image cropping operation.
type crop struct {
	Left   int `json:"left"`
	Right  int `json:"right"`
	Top    int `json:"top"`
	Bottom int `json:"bottom"`
}

// NewCrop creates a cropping operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewCrop(m json.RawMessage) (unsafe.Pointer, error) {
	op := crop{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// C does bound-snapping; all values accepted, no input validation required
	return unsafe.Pointer(C.Crp_New(
		C.int(op.Left),
		C.int(op.Right),
		C.int(op.Top),
		C.int(op.Bottom),
	)), nil
}

// drawTextBoxes represents a postprocessing operation which draws rectangles
// around detected text.
type drawTextBoxes struct {
	// Text box line color.
	Color [4]float32 `json:"color"`
	// Text box line thickness.
	Thickness int `json:"thickness"`
}

// NewDrawTextBoxes creates a drawTextBoxes operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewDrawTextBoxes(m json.RawMessage) (unsafe.Pointer, error) {
	op := drawTextBoxes{
		Color:     [4]float32{0, 0, 0, 0},
		Thickness: 2,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// all values accepted, no input validation required
	return unsafe.Pointer(C.DrawTB_New(
		(*C.float)(&op.Color[0]),
		C.int(op.Thickness),
	)), nil
}

// gaussianBlur represents an operation which applies Gaussian blur to an image.
type gaussianBlur struct {
	KernelWidth  int `json:"kernel_width"`
	KernelHeight int `json:"kernel_height"`
	// SigmaX is the kernel standard deviation in the x direction.
	SigmaX float32 `json:"sigma_x"`
	// SigmaY is the kernel standard deviation in the y direction.
	SigmaY float32 `json:"sigma_y"`
}

// NewGaussianBlur creates a gaussianBlur operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewGaussianBlur(m json.RawMessage) (unsafe.Pointer, error) {
	op := gaussianBlur{
		KernelWidth:  3,
		KernelHeight: 3,
		SigmaX:       0.0,
		SigmaY:       0.0,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.KernelWidth <= 0 || op.KernelWidth%2 != 1 {
		return nil, errors.New("ocr.NewGaussianBlur: bad kernel width")
	}
	if op.KernelHeight <= 0 || op.KernelHeight%2 != 1 {
		return nil, errors.New("ocr.NewGaussianBlur: bad kernel height")
	}
	// SigmaX and SigmaY accept all values
	return unsafe.Pointer(C.GaussBlur_New(
		C.int(op.KernelWidth),
		C.int(op.KernelHeight),
		C.float(op.SigmaX),
		C.float(op.SigmaY),
	)), nil
}

// invert represents a bitwise inversion operation.
type invert struct{}

// NewInvert creates an invert operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewInvert(m json.RawMessage) (unsafe.Pointer, error) {
	op := invert{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// no input validation required
	return unsafe.Pointer(C.Inv_New()), nil
}

// medianBlur represents an operation which applies Gaussian blur to an image.
type medianBlur struct {
	// KernelSize is the size of the blur kernel (stencil).
	// It should be an odd number.
	KernelSize int `json:"kernel_size"`
}

// NewMedianBlur creates a medianBlur operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewMedianBlur(m json.RawMessage) (unsafe.Pointer, error) {
	op := medianBlur{
		KernelSize: 3,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.KernelSize%2 != 1 {
		return nil, errors.New("ocr.NewMedianBlur: bad kernel size")
	}
	return unsafe.Pointer(C.MedBlur_New(C.int(op.KernelSize))), nil
}

// kernelType represents the available kernel (stencil) types.
type kernelType int

const (
	KernelRect kernelType = iota
	KernelCross
	KernelEllipse
)

func (t *kernelType) UnmarshalJSON(data []byte) error {
	var s string
	if err := json.Unmarshal(data, &s); err != nil {
		return err
	}
	switch s {
	default:
		return fmt.Errorf("bad kernel type: %q", s)
	case "rectangle":
		*t = KernelRect
	case "cross":
		*t = KernelCross
	case "ellipse":
		*t = KernelEllipse
	}
	return nil
}

func (t kernelType) MarshalJSON() ([]byte, error) {
	switch t {
	case KernelRect:
		return json.Marshal("rectangle")
	case KernelCross:
		return json.Marshal("cross")
	case KernelEllipse:
		return json.Marshal("ellipse")
	}
	return nil, fmt.Errorf("bad kernel type: %d", t)
}

// morphType represents the available morphology operation types.
type morphType int

const (
	MorphErode morphType = iota
	MorphDilate
	MorphOpen
	MorphClose
	MorphGradient
	MorphTopHat
	MorphBlackHat
	MorphHitMiss
)

func (t *morphType) UnmarshalJSON(data []byte) error {
	var s string
	if err := json.Unmarshal(data, &s); err != nil {
		return err
	}
	switch s {
	default:
		return fmt.Errorf("bad morphology type: %q", s)
	case "erode":
		*t = MorphErode
	case "dilate":
		*t = MorphDilate
	case "open":
		*t = MorphOpen
	case "close":
		*t = MorphClose
	case "gradient":
		*t = MorphGradient
	case "top_hat":
		*t = MorphTopHat
	case "black_hat":
		*t = MorphBlackHat
	case "hit_miss":
		*t = MorphHitMiss
	}
	return nil
}

func (t morphType) MarshalJSON() ([]byte, error) {
	switch t {
	case MorphErode:
		return json.Marshal("erode")
	case MorphDilate:
		return json.Marshal("dilate")
	case MorphOpen:
		return json.Marshal("open")
	case MorphClose:
		return json.Marshal("close")
	case MorphGradient:
		return json.Marshal("gradient")
	case MorphTopHat:
		return json.Marshal("top_hat")
	case MorphBlackHat:
		return json.Marshal("black_hat")
	case MorphHitMiss:
		return json.Marshal("hit_miss")
	}
	return nil, fmt.Errorf("bad morphology type: %d", t)
}

// morphology represents a morphology operation.
type morphology struct {
	KernelType   kernelType `json:"kernel_type"`
	KernelWidth  int        `json:"kernel_width"`
	KernelHeight int        `json:"kernel_height"`
	Type         morphType  `json:"type"`
	// Iterations is the number of morphology iterations to be applied.
	Iterations int `json:"iterations"`
}

// NewMorphology creates a morphology operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewMorphology(m json.RawMessage) (unsafe.Pointer, error) {
	op := morphology{
		KernelType:   KernelRect,
		KernelWidth:  3,
		KernelHeight: 3,
		Type:         MorphOpen,
		Iterations:   1,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.KernelWidth <= 0 {
		return nil, errors.New("ocr.NewMorphology: bad kernel width")
	}
	if op.KernelHeight <= 0 {
		return nil, errors.New("ocr.NewMorphology: bad kernel height")
	}
	// Iterations accepts all values; types are valid if parsed
	return unsafe.Pointer(C.Morph_New(
		C.int(op.KernelType),
		C.int(op.KernelWidth),
		C.int(op.KernelHeight),
		C.int(op.Type),
		C.int(op.Iterations),
	)), nil
}

// normBC represents an automatic brigthness and contrast adjustment operation.
type normBC struct {
	// Clip percentage.
	ClipPct float32 `json:"clip_pct"`
}

// NewNormBC creates a normBC operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewNormBC(m json.RawMessage) (unsafe.Pointer, error) {
	op := normBC{
		ClipPct: 0.5,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// all values accepted, no input validation required
	return unsafe.Pointer(C.NormBC_New(
		C.float(op.ClipPct),
	)), nil
}

// resize represents an image resizing operation.
type resize struct {
	Width  int `json:"width"`
	Height int `json:"height"`
}

// NewResize creates a resize operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewResize(m json.RawMessage) (unsafe.Pointer, error) {
	op := resize{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.Width <= 0 {
		return nil, errors.New("ocr.NewResize: bad width")
	}
	if op.Height <= 0 {
		return nil, errors.New("ocr.NewResize: bad height")
	}
	return unsafe.Pointer(C.Rsz_New(
		C.int(op.Width),
		C.int(op.Height),
	)), nil
}

// rotate represents an image rotation operation.
type rotate struct {
	Angle float32 `json:"angle"`
}

// NewRotate creates a rotation operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewRotate(m json.RawMessage) (unsafe.Pointer, error) {
	op := rotate{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// all values accepted, no input validation required
	return unsafe.Pointer(C.Rot_New(
		C.float(op.Angle),
	)), nil
}

// threshType represents the available types of threshold operations.
type threshType int

const (
	ThreshBinary threshType = iota
	ThreshBinaryInv
	ThreshTrunc
	ThreshToZero
	ThreshToZeroInv
	ThreshOtsu     = 8
	ThreshTriangle = 16
)

func (t *threshType) UnmarshalJSON(data []byte) error {
	var s string
	if err := json.Unmarshal(data, &s); err != nil {
		return err
	}
	switch s {
	default:
		return fmt.Errorf("bad threshold type: %q", s)
	case "binary":
		*t = ThreshBinary
	case "binary_inv":
		*t = ThreshBinaryInv
	case "trunc":
		*t = ThreshTrunc
	case "to_zero":
		*t = ThreshToZero
	case "to_zero_inv":
		*t = ThreshToZeroInv
	case "otsu":
		*t = ThreshOtsu
	case "triangle":
		*t = ThreshTriangle
	}
	return nil
}

func (t threshType) MarshalJSON() ([]byte, error) {
	switch t {
	case ThreshBinary:
		return json.Marshal("binary")
	case ThreshBinaryInv:
		return json.Marshal("binary_inv")
	case ThreshTrunc:
		return json.Marshal("trunc")
	case ThreshToZero:
		return json.Marshal("to_zero")
	case ThreshToZeroInv:
		return json.Marshal("to_zero_inv")
	case ThreshOtsu:
		return json.Marshal("otsu")
	case ThreshTriangle:
		return json.Marshal("triangle")
	}
	return nil, fmt.Errorf("bad threshold type: %d", t)
}

// threshold represents a thresholding operation.
type threshold struct {
	Value    float32 `json:"value"`
	MaxValue float32 `json:"max_value"`
	// Type is the threshold type.
	// Since either Otsu or triangle thresholds can be added to/used with
	// any of the basic threshold types, it is a slice, because parsing
	// arithmetic from JSON is hard.
	Type [2]threshType `json:"type"`
}

// NewThreshold creates a threshold operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewThreshold(m json.RawMessage) (unsafe.Pointer, error) {
	op := threshold{
		Value:    0.0,
		MaxValue: 255.0,
		Type:     [2]threshType{ThreshBinary, ThreshOtsu},
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// if 2 types are specified, 1 must be either ThreshOtsu or ThreshTriangle
	typ := op.Type[0] + op.Type[1]
	if !(typ >= ThreshBinary && typ <= ThreshToZeroInv) &&
		!(typ >= ThreshOtsu && typ <= ThreshOtsu+ThreshToZeroInv) &&
		!(typ >= ThreshTriangle && typ <= ThreshTriangle+ThreshToZeroInv) {
		return nil, errors.New("ocr.NewThreshold: bad threshold type")
	}
	// Value and MaxValue accept all values
	return unsafe.Pointer(C.Thresh_New(
		C.float(op.Value),
		C.float(op.MaxValue),
		C.int(op.Type[0]+op.Type[1]),
	)), nil
}
