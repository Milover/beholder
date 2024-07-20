package ocr

/*
#include <stdlib.h>
#include "ocr.h"
*/
import "C"
import (
	"io"

	"github.com/Milover/beholder/internal/stopwatch"
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
	T *Tesseract      `json:"tesseract"`
	P *ImageProcessor `json:"image_processing"`
	O *Output         `json:"output"`
}

// NewOCR constructs a new OCR.
// WARNING: since OCR's fields allocate C-memory,
// Delete must be called to release memory when no longer needed.
func NewOCR() OCR {
	return OCR{
		T: NewTesseract(),
		P: NewImageProcessor(),
		O: NewOutput(),
	}
}

// Delete releases C-allocated memory by calling each field's Delete method.
// Once called, the contents (fields) of o are no longer valid.
func (o OCR) Delete() {
	o.T.Delete()
	o.P.Delete()
}

// Finalize frees all of o's resources, closes all open files and/or connections
// and flushes the output buffer.
// Once called, o is no longer valid.
func (o OCR) Finalize() error {
	o.Delete()
	return o.O.Close()
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
	if err := o.O.Init(); err != nil {
		return err
	}
	return nil
}

// Run is a function that runs the OCR pipeline for a single image: reading,
// preprocessing, recognition and postprocessing.
func (o OCR) Run(r io.Reader) (Result, error) {
	sw := stopwatch.New()
	res := Result{TimeStamp: sw.Start}

	// FIXME: this should probably happen in a different goroutine
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
