package cmd

import (
	"fmt"
	"strings"
	"time"

	"github.com/Milover/beholder/internal/chrono"
	"github.com/Milover/beholder/internal/models"
)

// Stats holds various program statistics.
type Stats struct {
	Result *models.Result
	// AvgTimings is a map of averaged [models.Result.Timings]
	AvgTimings chrono.Timings
	// InitDuration is the time elapsed while initializing the OCR pipeline.
	InitDuration time.Duration
	// ExecDuration is the total time elapsed while running the program.
	ExecDuration time.Duration
}

// NewStats creates a new ready to use Stats.
func NewStats() *Stats {
	return &Stats{
		Result: models.NewResult(),
	}
}

// Accumulate is a function which accumulates timings.
//
// FIXME: this is a stupid function and the way we're doing this now makes
// no sense.
func (s *Stats) Accumulate(ts chrono.Timings) {
	if len(s.AvgTimings) == 0 {
		s.AvgTimings = make([]chrono.Timing, 0, len(ts))
		s.AvgTimings = append(s.AvgTimings, ts...)
	} else {
		// BUG: need bounds checking here
		for i := range ts {
			s.AvgTimings[i].Value += ts[i].Value
		}
	}
}

// Average is a function which computes timing averages for count timings.
func (s *Stats) Average(count int64) {
	for i := range s.AvgTimings {
		s.AvgTimings[i].Value /= time.Duration(count)
	}
}

func (s *Stats) String() string {
	var b strings.Builder
	fmt.Fprintf(&b, "%v\ninit: %v\nexec: %v\n",
		s.AvgTimings,
		s.InitDuration,
		s.ExecDuration,
	)
	return b.String()
}
