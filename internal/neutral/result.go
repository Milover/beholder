package neutral

import (
	"time"

	"github.com/Milover/beholder/internal/chrono"
	"github.com/Milover/beholder/internal/enumutils"
)

// Result holds the processing pipeline results and statistics.
type Result struct {
	// Text is a list of strings associated with each Box.
	Text []string `json:"result"`
	// Expected is the expected Text.
	Expected []string `json:"expected"`
	// Confidence is a list of confidences associated with
	// each Box or Text string.
	Confidence []float64 `json:"confidence"`
	// Boxes is a list of bounding boxes detected by the processing pipeline.
	Boxes []Rectangle `json:"boxes"`
	// Status is the final status of the result.
	Status ResultStatus `json:"status"`
	// TimeStamp is the time at which processing started.
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
	r.Confidence = r.Confidence[:0]
	r.Boxes = r.Boxes[:0]
	r.Status = RSNone
	r.TimeStamp = time.Time{}
	r.Timings.Reset()
}

// ResultStatus designates whether the result of the OCR pipeline is
// good, bad or unclear.
type ResultStatus int

const (
	RSNone ResultStatus = iota
	// The recognized text matches the expected text.
	RSPass
	// The recognized text does not match the expected text, or
	// there was an error during OCR pipeline execution.
	RSFail
	// The exact state of the result is not clear, eg. the recognized text
	// matches the expected text, but there were issues, or the expected
	// text is unknown.
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
