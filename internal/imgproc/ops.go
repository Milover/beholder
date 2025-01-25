// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package imgproc

/*
#include <stdlib.h>
#include "ops.h"
*/
import "C"
import (
	"encoding/json"
	"errors"
	"unsafe"

	"github.com/Milover/beholder/internal/enumutils"
)

// opFactory is a function which creates a new image processing operation
// using a mix of default values and runtime supplied data.
// WARNING: the operation is C-allocated and will be managed by C,
// hence C.free should NOT be called on the returned pointer.
type opFactory func(json.RawMessage) (unsafe.Pointer, error)

// opFactoryMap maps the names of image processing operations to their
// factory functions.
var opFactoryMap = map[string]opFactory{
	"add_padding":                   NewAddPadding,
	"adaptive_threshold":            NewAdaptiveThreshold,
	"auto_crop":                     NewAutoCrop,
	"auto_orient":                   NewAutoOrient,
	"bgr":                           NewBGR,
	"clahe":                         NewCLAHE,
	"correct_gamma":                 NewCorrectGamma,
	"crop":                          NewCrop,
	"deblur":                        NewDeblur,
	"div_gaussian_blur":             NewDivGaussianBlur,
	"draw_bounding_boxes":           NewDrawBoundingBoxes,
	"draw_labels":                   NewDrawLabels,
	"equalize_histogram":            NewEqualizeHistogram,
	"fast_nl_means_denoise":         NewFastNlMeansDenoise,
	"gaussian_blur":                 NewGaussianBlur,
	"grayscale":                     NewGrayscale,
	"invert":                        NewInvert,
	"landscape":                     NewLandscape,
	"median_blur":                   NewMedianBlur,
	"morphology":                    NewMorphology,
	"normalize_brightness_contrast": NewNormBC,
	"rescale":                       NewRescale,
	"resize":                        NewResize,
	"resize_to_height":              NewResizeToHeight,
	"rotate":                        NewRotate,
	"threshold":                     NewThreshold,
	"unsharp_mask":                  NewUnsharpMask,
}

// addPadding adds uniform (white) padding to the border of an image.
type addPadding struct {
	Padding  int     `json:"padding"`
	PadValue float64 `json:"pad_value"`
}

// NewAddPadding creates a new border padding operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewAddPadding(m json.RawMessage) (unsafe.Pointer, error) {
	op := addPadding{
		Padding: 10,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.PadValue < 0 {
		return nil, errors.New("imgproc.NewAddPadding: bad pad value")
	}
	return unsafe.Pointer(C.AdPad_New(
		C.int(op.Padding),
		C.double(op.PadValue),
	)), nil
}

// adThreshType is an adaptive threshold type.
type adThreshType int

const (
	AdThreshMean adThreshType = iota
	AdThreshGaussian
)

var (
	adThreshTypeMap = map[adThreshType]string{
		AdThreshMean:     "mean",
		AdThreshGaussian: "gaussian",
	}
	invAdThreshTypeMap = enumutils.Invert(adThreshTypeMap)
)

func (t *adThreshType) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, t, invAdThreshTypeMap)
}

func (t adThreshType) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(t, adThreshTypeMap)
}

// adaptiveThreshold is an adaptive thresholding operation.
type adaptiveThreshold struct {
	// MaxValue is the value assigned to pixels satisfying the thresholding
	// condition.
	MaxValue float64 `json:"max_value"`
	// Size is the pixel neighbourhood used to compute the threshold value
	// for a pixel: 3, 5, 7...
	KernelSize int `json:"kernel_size"`
	// Const is a constant subtracted from the mean.
	Const float64 `json:"const"`
	// Type is the adaptive thresholding algorithm type.
	Type adThreshType `json:"type"`
}

// NewAdaptiveThreshold creates an adaptive threshold operation with default
// values, unmarshals runtime data into it and then constructs a C-class
// representing the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewAdaptiveThreshold(m json.RawMessage) (unsafe.Pointer, error) {
	op := adaptiveThreshold{
		MaxValue:   255.0,
		KernelSize: 11,  // magic
		Const:      2.0, // magic
		Type:       AdThreshGaussian,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.MaxValue < 0.0 || op.MaxValue > 255.01 {
		return nil, errors.New("imgproc.NewAdaptiveThreshold: bad max value")
	}
	if op.KernelSize < 3 || op.KernelSize%2 != 1 {
		return nil, errors.New("imgproc.NewAdaptiveThreshold: bad kernel size")
	}
	return unsafe.Pointer(C.AdThresh_New(
		C.double(op.MaxValue),
		C.int(op.KernelSize),
		C.double(op.Const),
		C.int(op.Type),
	)), nil
}

// autoCrop represents an automatic image cropping operation.
// It currently uses morphologic gradients to detect text boxes and
// then orients and crops the image.
type autoCrop struct {
	autoOrient // all fields are the same
}

// NewAutoCrop creates an automatic cropping operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewAutoCrop(m json.RawMessage) (unsafe.Pointer, error) {
	op := autoCrop{
		autoOrient: autoOrient{
			KernelSize: 50,
			TextWidth:  50,
			TextHeight: 50,
			Padding:    10,
			PadValue:   255.0,
		},
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.KernelSize <= 0 {
		return nil, errors.New("imgproc.NewAutoCrop: bad kernel size")
	}
	if op.PadValue < 0 {
		return nil, errors.New("imgproc.NewAutoCrop: bad pad value")
	}
	// Padding, TextWidth and TextHeight accept all values
	return unsafe.Pointer(C.AuCrp_New(
		C.int(op.KernelSize),
		C.float(op.TextWidth),
		C.float(op.TextHeight),
		C.float(op.Padding),
		C.double(op.PadValue),
	)), nil
}

// autoOrient represents an automatic image orientation operation.
// It currently uses morphologic gradients to detect text boxes and
// then orients the image.
type autoOrient struct {
	// KernelSize is the size of the kernel used for connecting text blobs.
	KernelSize int `json:"kernel_size"`
	// TextWidth is the minium width of the blob for it to be considered text.
	TextWidth float32 `json:"text_width"`
	// TextHeight is the minium height of the blob for it to be considered text.
	TextHeight float32 `json:"text_height"`
	// Padding is the additional padding added to the detected text box.
	Padding float32 `json:"padding"`
	// PadValue is the pixel value used for padding.
	PadValue float64 `json:"pad_value"`
}

// bgr converts a grayscale image to a 3-channel BGR image.
type bgr struct{}

// NewBGR creates a bgr operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewBGR(m json.RawMessage) (unsafe.Pointer, error) {
	op := bgr{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// no input validation required
	return unsafe.Pointer(C.BGR_New()), nil
}

// NewAutoOrient creates an automaticcropping operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewAutoOrient(m json.RawMessage) (unsafe.Pointer, error) {
	op := autoOrient{
		KernelSize: 50,
		TextWidth:  50,
		TextHeight: 50,
		Padding:    10,
		PadValue:   255.0,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.KernelSize <= 0 {
		return nil, errors.New("imgproc.NewAutoOrient: bad kernel size")
	}
	if op.PadValue < 0 {
		return nil, errors.New("imgproc.NewAutoOrient: bad pad value")
	}
	// Padding, TextWidth and TextHeight accept all values
	return unsafe.Pointer(C.AuOrien_New(
		C.int(op.KernelSize),
		C.float(op.TextWidth),
		C.float(op.TextHeight),
		C.float(op.Padding),
		C.double(op.PadValue),
	)), nil
}

// clahe represents a contrast limited adaptive histogram equalization operation.
type clahe struct {
	// ClipLimit is the threshold value for contrast limiting.
	ClipLimit float32 `json:"clip_limit"`
	// TileRows is the number of tile columns.
	TileRows int `json:"tile_rows"`
	// TileColumns is the number of tile rows.
	TileColumns int `json:"tile_columns"`
}

// NewCLAHE creates a new contrast limited adaptive histogram equalization
// operation with default values, unmarshals runtime data into it and then
// constructs a C-class representing the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewCLAHE(m json.RawMessage) (unsafe.Pointer, error) {
	op := clahe{
		ClipLimit:   40.0,
		TileRows:    8,
		TileColumns: 8,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.TileRows <= 0 {
		return nil, errors.New("imgproc.NewCLAHE: bad tile rows")
	}
	if op.TileColumns <= 0 {
		return nil, errors.New("imgproc.NewCLAHE: bad tile columns")
	}
	// ClipLimit accepts all values
	return unsafe.Pointer(C.CLH_New(
		C.float(op.ClipLimit),
		C.int(op.TileRows),
		C.int(op.TileColumns),
	)), nil
}

// correctGamma performs gamma correction on an image.
type correctGamma struct {
	Gamma float64 `json:"gamma"`
}

// NewCorrectGamma creates a gamma correction operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewCorrectGamma(m json.RawMessage) (unsafe.Pointer, error) {
	op := correctGamma{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.Gamma < 0.0 {
		return nil, errors.New("imgproc.NewCorrectGamma: bad gamma value")
	}
	return unsafe.Pointer(C.CorrGamma_New(
		C.double(op.Gamma),
	)), nil
}

// crop represents an image cropping operation.
type crop struct {
	Left   int `json:"left"`
	Top    int `json:"top"`
	Width  int `json:"width"`
	Height int `json:"height"`
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
		C.int(op.Top),
		C.int(op.Width),
		C.int(op.Height),
	)), nil
}

// deblur represents an out-of-focus image deblurring operation.
// See the [OpenCV example] for more information.
//
// [OpenCV example]: https://docs.opencv.org/4.10.0/de/d3c/tutorial_out_of_focus_deblur_filter.html
type deblur struct {
	Radius int `json:"radius"` // point spread function (PSF) radius
	SNR    int `json:"snr"`    // signal-to-noise ratio
}

// NewDeblur creates a deblurring operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewDeblur(m json.RawMessage) (unsafe.Pointer, error) {
	op := deblur{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.Radius <= 0 {
		return nil, errors.New("imgproc.NewDeblur: bad radius")
	}
	if op.SNR <= 0 {
		return nil, errors.New("imgproc.NewDeblur: bad signal-to-noise ratio")
	}
	return unsafe.Pointer(C.Dblr_New(
		C.int(op.Radius),
		C.int(op.SNR),
	)), nil
}

// divGaussianBlur represents an operation which applies background removal
// by dividing the image with a Gaussian filtered and scaled version of itself.
//
// TODO: this should just be a division operation, and we should be able
// to divide with 'a matrix', instead of re-implementing every possible
// operation. This is fine for now, though.
type divGaussianBlur struct {
	// ScaleFactor scales the filtered image, usually by 255.
	ScaleFactor float32 `json:"scale_factor"`
	// SigmaX is the Gaussian filter kernel standard deviation
	// in the x direction, usually some non-zero value.
	SigmaX float32 `json:"sigma_x"`
	// SigmaY is the Gaussian filter kernel standard deviation
	// in the y direction, usually some non-zero value.
	SigmaY float32 `json:"sigma_y"`
	// KernelWidth is the Gaussian filter kernel width, usually 0.
	KernelWidth int `json:"kernel_width"`
	// KernelHeight is the Gaussian filter kernel height, usually 0.
	KernelHeight int `json:"kernel_height"`
}

// NewDivGaussianBlur creates a divGaussianBlur operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewDivGaussianBlur(m json.RawMessage) (unsafe.Pointer, error) {
	op := divGaussianBlur{
		ScaleFactor:  255.0,
		SigmaX:       0.0,
		SigmaY:       0.0,
		KernelWidth:  0,
		KernelHeight: 0,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.SigmaX <= 0 && (op.KernelWidth < 0 || op.KernelWidth%2 != 1) {
		return nil, errors.New("imgproc.NewDivGaussianBlur: bad kernel width")
	}
	if op.SigmaY <= 0 && (op.KernelHeight < 0 || op.KernelHeight%2 != 1) {
		return nil, errors.New("imgproc.NewDivGaussianBlur: bad kernel height")
	}
	// SigmaX and SigmaY accept all values
	return unsafe.Pointer(C.DivGaussBlur_New(
		C.float(op.ScaleFactor),
		C.float(op.SigmaX),
		C.float(op.SigmaY),
		C.int(op.KernelWidth),
		C.int(op.KernelHeight),
	)), nil
}

// drawBoundingBoxes represents a postprocessing operation which draws rectangles
// around detected text.
type drawBoundingBoxes struct {
	// Text box line color.
	Color [4]float32 `json:"color"`
	// Text box line thickness.
	Thickness int `json:"thickness"`
}

// NewDrawBoundingBoxes creates a drawTextBoxes operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewDrawBoundingBoxes(m json.RawMessage) (unsafe.Pointer, error) {
	op := drawBoundingBoxes{
		Color:     [4]float32{0, 0, 0, 0},
		Thickness: 2,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// all values accepted, no input validation required
	return unsafe.Pointer(C.DrawBB_New(
		(*C.float)(&op.Color[0]),
		C.int(op.Thickness),
	)), nil
}

// drawLabels represents a postprocessing operation which draws text labels
// at the top left corner of the object bounding box.
type drawLabels struct {
	Color     [4]float32 `json:"color"`      // font color
	FontScale float64    `json:"font_scale"` // font size multiplier
	Thickness int        `json:"thickness"`  // font line thickness
}

// NewDrawLabels creates a drawLabels operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewDrawLabels(m json.RawMessage) (unsafe.Pointer, error) {
	op := drawLabels{
		Color:     [4]float32{0, 0, 0, 0},
		FontScale: 1.0,
		Thickness: 2,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.FontScale <= 0 {
		return nil, errors.New("imgproc.NewDrawLabels: bad font scale")
	}
	if op.Thickness <= 0 {
		return nil, errors.New("imgproc.NewDrawLabels: bad thickness")
	}
	return unsafe.Pointer(C.DrawLbl_New(
		(*C.float)(&op.Color[0]),
		C.double(op.FontScale),
		C.int(op.Thickness),
	)), nil
}

// equalizeHistogram represents a global image histogram equalization operation.
type equalizeHistogram struct{}

// fastNlMeansDenoise is a denoising operation using the
// Non-local Means Denoising algorithm.
//
// [Non-local Means Denoising]: https://docs.opencv.org/4.10.0/d1/d79/group__photo__denoise.html#ga0db3ea0715152e56081265014b139bec
type fastNlMeansDenoise struct {
	// Weight is the parameter regulating the filter strength.
	// Bigger values remove more noise and more image detail, smaller values
	// preserve more detail and noise.
	//
	// A loose guideline for typical values is the range [5, 15].
	Weight float32 `json:"weight"`
}

// NewFastNlMeansDenoise creates a Non-local Means Denoising operation with
// default values, unmarshals runtime data into it and then constructs
// a C-class representing  the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewFastNlMeansDenoise(m json.RawMessage) (unsafe.Pointer, error) {
	op := fastNlMeansDenoise{
		Weight: 10,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.Weight <= 0 {
		return nil, errors.New("imgproc.NewFastNlMeansDenoise: bad weight")
	}
	return unsafe.Pointer(C.FNlMDenoise_New(C.float(op.Weight))), nil
}

// NewEqualizeHistogram creates a global image histogram equalization operation
// with default values, unmarshals runtime data into it and then
// constructs a C-class representing the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewEqualizeHistogram(m json.RawMessage) (unsafe.Pointer, error) {
	op := equalizeHistogram{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// no input validation required
	return unsafe.Pointer(C.EqHist_New()), nil
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
	if op.SigmaX <= 0 && (op.KernelWidth < 0 || op.KernelWidth%2 != 1) {
		return nil, errors.New("imgproc.NewGaussianBlur: bad kernel width")
	}
	if op.SigmaY <= 0 && (op.KernelHeight < 0 || op.KernelHeight%2 != 1) {
		return nil, errors.New("imgproc.NewGaussianBlur: bad kernel height")
	}
	// SigmaX and SigmaY accept all values
	return unsafe.Pointer(C.GaussBlur_New(
		C.int(op.KernelWidth),
		C.int(op.KernelHeight),
		C.float(op.SigmaX),
		C.float(op.SigmaY),
	)), nil
}

// grayscale converts any color image to a single-channel gray scale image.
type grayscale struct{}

// NewGrayscale creates a grayscale operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewGrayscale(m json.RawMessage) (unsafe.Pointer, error) {
	op := grayscale{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// no input validation required
	return unsafe.Pointer(C.Gray_New()), nil
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

// landscape is an operation which, optionally, rotates the image 90° clockwise
// so that image width is greater than image height.
// Does nothing if image width is already greater than image height.
type landscape struct{}

// NewLandscape creates a landscape operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewLandscape(m json.RawMessage) (unsafe.Pointer, error) {
	op := landscape{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// no input validation required
	return unsafe.Pointer(C.LScape_New()), nil
}

// medianBlur represents an operation which applies median blur to an image.
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
		return nil, errors.New("imgproc.NewMedianBlur: bad kernel size")
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

var (
	kernelTypeMap = map[kernelType]string{
		KernelRect:    "rectangle",
		KernelCross:   "cross",
		KernelEllipse: "ellipse",
	}
	invKernelTypeMap = enumutils.Invert(kernelTypeMap)
)

func (t *kernelType) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, t, invKernelTypeMap)
}

func (t kernelType) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(t, kernelTypeMap)
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

var (
	morphTypeMap = map[morphType]string{
		MorphErode:    "erode",
		MorphDilate:   "dilate",
		MorphOpen:     "open",
		MorphClose:    "close",
		MorphGradient: "gradient",
		MorphTopHat:   "top_hat",
		MorphBlackHat: "black_hat",
		MorphHitMiss:  "hit_miss",
	}
	invMorphTypeMap = enumutils.Invert(morphTypeMap)
)

func (t *morphType) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, t, invMorphTypeMap)
}

func (t morphType) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(t, morphTypeMap)
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
		return nil, errors.New("imgproc.NewMorphology: bad kernel width")
	}
	if op.KernelHeight <= 0 {
		return nil, errors.New("imgproc.NewMorphology: bad kernel height")
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

// normBC represents an automatic brightness and contrast adjustment operation.
type normBC struct {
	ClipLowPct  float32 `json:"clip_low_pct"`  // low-value side clip percentage
	ClipHighPct float32 `json:"clip_high_pct"` // high-value side clip percentage
}

// NewNormBC creates a normBC operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewNormBC(m json.RawMessage) (unsafe.Pointer, error) {
	op := normBC{
		ClipLowPct:  0.25,
		ClipHighPct: 0.25,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.ClipLowPct < 0 || op.ClipLowPct > 100 {
		return nil, errors.New("imgproc.NewNormBC: bad clip-low percentage")
	}
	if op.ClipHighPct < 0 || op.ClipHighPct > 100 {
		return nil, errors.New("imgproc.NewNormBC: bad clip-high percentage")
	}
	// all values accepted, no input validation required
	return unsafe.Pointer(C.NormBC_New(
		C.float(op.ClipLowPct),
		C.float(op.ClipHighPct),
	)), nil
}

// rescale represents an image rescaling operation.
type rescale struct {
	Scale float64 `json:"scale"`
}

// NewRescale creates a rescaling operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewRescale(m json.RawMessage) (unsafe.Pointer, error) {
	op := rescale{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.Scale <= 0 {
		return nil, errors.New("imgproc.NewRescale: bad scale")
	}
	return unsafe.Pointer(C.Rsl_New(
		C.double(op.Scale),
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
		return nil, errors.New("imgproc.NewResize: bad width")
	}
	if op.Height <= 0 {
		return nil, errors.New("imgproc.NewResize: bad height")
	}
	return unsafe.Pointer(C.Rsz_New(
		C.int(op.Width),
		C.int(op.Height),
	)), nil
}

// resizeToHeight resizes the image to the specified height while keeping
// the original aspect ratio.
type resizeToHeight struct {
	Height int `json:"height"`
}

// NewResizeToHeight creates a resize operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewResizeToHeight(m json.RawMessage) (unsafe.Pointer, error) {
	op := resizeToHeight{}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	if op.Height <= 0 {
		return nil, errors.New("imgproc.NewResizeToHeight: bad height")
	}
	return unsafe.Pointer(C.RszToH_New(
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

var (
	threshTypeMap = map[threshType]string{
		ThreshBinary:    "binary",
		ThreshBinaryInv: "binary_inv",
		ThreshTrunc:     "trunc",
		ThreshToZero:    "to_zero",
		ThreshToZeroInv: "to_zero_inv",
		ThreshOtsu:      "otsu",
		ThreshTriangle:  "triangle",
	}
	invThreshTypeMap = enumutils.Invert(threshTypeMap)
)

func (t *threshType) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, t, invThreshTypeMap)
}

func (t threshType) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(t, threshTypeMap)
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
		return nil, errors.New("imgproc.NewThreshold: bad threshold type")
	}
	// Value and MaxValue accept all values
	return unsafe.Pointer(C.Thresh_New(
		C.float(op.Value),
		C.float(op.MaxValue),
		C.int(op.Type[0]+op.Type[1]),
	)), nil
}

// unsharpMask performs image sharpening using the "unsharp mask" algorithm.
// See the following resources for more information:
//   - [OpenCV example] (the current implementation)
//   - [stackoverflow thread] on image sharpening
//   - [Digital unsharp masking] on Wikipedia
//
// [OpenCV example]: https://docs.opencv.org/4.10.0/d1/d10/classcv_1_1MatExpr.html#details
// [stackoverflow thread]: https://stackoverflow.com/questions/4993082/how-can-i-sharpen-an-image-in-opencv
// [Digital unsharp masking]: https://en.wikipedia.org/wiki/Unsharp_masking#Digital_unsharp_masking
type unsharpMask struct {
	Sigma     float64 `json:"sigma"`
	Threshold float64 `json:"threshold"`
	Amount    float64 `json:"amount"`
}

// NewUnsharpMask creates a unsharp mask operation with default values,
// unmarshals runtime data into it and then constructs a C-class representing
// the operation.
// WARNING: the C-allocated memory will be managed by C,
// hence C.free should NOT be called on the returned pointer.
func NewUnsharpMask(m json.RawMessage) (unsafe.Pointer, error) {
	op := unsharpMask{
		Sigma:     1.0,
		Threshold: 5.0,
		Amount:    1.0,
	}
	if err := json.Unmarshal(m, &op); err != nil {
		return nil, err
	}
	// FIXME: check values
	return unsafe.Pointer(C.UnshMsk_New(
		C.double(op.Sigma),
		C.double(op.Threshold),
		C.double(op.Amount),
	)), nil
}
