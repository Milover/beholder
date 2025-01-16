// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package stopwatch

import "time"

// Stopwatch emulates  a simple stopwatch.
// It's main use is to simplify successive time measurements.
type Stopwatch struct {
	// Start is the time at which the stopwatch started measuring time.
	Start time.Time
	// duration is the time elapsed from when the stopwatch started to
	// the last time Lap() was called.
	duration time.Duration
}

// New returns a new Stopwatch ready to be used.
func New() Stopwatch {
	return Stopwatch{
		Start:    time.Now(),
		duration: time.Duration(0),
	}
}

// Lap returns the time elapsed of the current lap, i.e. the time since
// Lap() was last called, or the start time, if this is the first lap.
func (s *Stopwatch) Lap() time.Duration {
	d := time.Since(s.Start)
	lap := d - s.duration
	s.duration = d
	return lap
}

// Reset resets the stopwatch and sets the start time to time.Now().
func (s *Stopwatch) Reset() {
	s.Start = time.Now()
	s.duration = time.Duration(0)
}

// Total returns the time elapsed since the stopwatch was started
func (s Stopwatch) Total() time.Duration {
	return time.Since(s.Start)
}
