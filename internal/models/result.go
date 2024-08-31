package models

import (
	"time"

	"github.com/Milover/beholder/internal/chrono"
	"github.com/Milover/beholder/internal/enumutils"
)

// Result holds results and statistics of a processing pipeline.
//
// TODO: a Result should probably know to which image it is associated, eg.
// by holding some type of reference to it.
type Result struct {
	// Boxes is a list of bounding boxes detected by a processing pipeline.
	Boxes []Rectangle `json:"boxes"`
	// Text is a list of strings associated with each Box.
	//
	// A Text string could, for example, be the text recognized by OCR or
	// a object class tag assigned by an object detector.
	Text []string `json:"result"`
	// Expected is the expected Text.
	Expected []string `json:"expected"`
	// Confidences is a list of confidences associated with each Result item.
	//
	// As an example, for a Result produced by OCR, the confidence could
	// be the confidence in the recognized Text string.
	Confidences []float64 `json:"confidences"`
	// Status is the final status of the result.
	Status ResultStatus `json:"status"`
	// TimeStamp is the time at which the result was created, which is usually
	// the time at which the processing pipeline started.
	TimeStamp time.Time `json:"timestamp"`
	// Timings are the named durations of various parts of the processing
	// pipeline.
	Timings chrono.Timings `json:"-"`
}

// NewResult creates a new ready to use Result.
func NewResult() *Result {
	return &Result{}
}

// Reset resets all fields of r, but retains allocated memory.
func (r *Result) Reset() {
	r.Text = r.Text[:0]
	r.Expected = r.Expected[:0]
	r.Confidences = r.Confidences[:0]
	r.Boxes = r.Boxes[:0]
	r.Status = RSNone
	r.TimeStamp = time.Time{}
	r.Timings.Reset()
}

// ResultStatus indicates the (final) state of a processing pipeline [Result].
type ResultStatus int

const (
	// RSNone is the zero-value ResultStatus.
	RSNone ResultStatus = iota
	// RSPass indicates that the obtained [Result] is the expected one.
	RSPass
	// RSFail indicates that the obtained [Result] is not the expected one.
	// For example, for a [Result] produced by OCR, it indicates that
	// the recognized text does not match the expected text, or that
	// there was an error during OCR.
	RSFail
	// RSInspect indicates that the state of the obtained [Result] is unclear.
	// For example, for a [Result] produced by OCR, it indicates that
	// that the expected text is unknown (not set), or that the recognized
	// text matches the expected text, but there were issues during OCR.
	RSInspect
)

var (
	resultStatusMap = map[ResultStatus]string{
		RSNone:    "none",
		RSPass:    "pass",
		RSFail:    "fail",
		RSInspect: "inspect",
	}
	invResultStatusMap = enumutils.Invert(resultStatusMap)
)

func (r ResultStatus) String() string {
	return resultStatusMap[r]
}

func (r *ResultStatus) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, r, invResultStatusMap)
}

func (r ResultStatus) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(r, resultStatusMap)
}
