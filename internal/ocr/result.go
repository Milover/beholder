package ocr

import (
	"time"

	"github.com/Milover/ocr/internal/enumutils"
)

// Result holds the OCR pipeline results and statistics.
type Result struct {
	// Text is the text recognized by the OCR.
	Text string `json:"result"`
	// Expected is the expected result of the OCR pipeline.
	Expected string `json:"expected"`
	// Status is the final status of the result.
	Status ResultStatus `json:"status"`
	// TimeStamp is the time at which the OCR pipeline started.
	TimeStamp time.Time `json:"timestamp"`
	// RunDuration is the time elapsed from start to finish of the OCR pipeline.
	RunDuration      time.Duration
	ReadDuration     time.Duration
	DecodeDuration   time.Duration
	PreprocDuration  time.Duration
	OCRDuration      time.Duration
	PostprocDuration time.Duration
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
