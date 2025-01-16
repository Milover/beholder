// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package cmd

import (
	"fmt"
	"math"
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

	avgCount int64 // rolling average count
}

// NewStats creates a new ready to use Stats.
func NewStats() *Stats {
	return &Stats{
		Result: models.NewResult(),
	}
}

// RollingAverage updates (computes) the rolling average with a ts.
func (s *Stats) RollingAverage(ts chrono.Timings) {
	switch l := len(s.AvgTimings); {
	// if this is the first call, or if ts contains new timings,
	// reset the average
	case l == 0, l < len(ts):
		s.AvgTimings = make([]chrono.Timing, len(ts))
		copy(s.AvgTimings, ts)
		s.avgCount = 1
		fallthrough
	case l == len(ts):
		s.avgCount++
		for i := range ts {
			dif := float64(ts[i].Value - s.AvgTimings[i].Value)
			s.AvgTimings[i].Value += time.Duration(math.Round(dif / float64(s.avgCount)))
		}
	}
}

func (s *Stats) String() string {
	var b strings.Builder
	fmt.Fprintf(&b, "\n%v\ninit: %v\nexec: %v\n",
		s.AvgTimings,
		s.InitDuration,
		s.ExecDuration,
	)
	return b.String()
}
