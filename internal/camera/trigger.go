// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package camera

/*
#include <stdlib.h>
#include "camera.h"
*/
import "C"
import (
	"errors"
	"time"

	"github.com/Milover/beholder/internal/chrono"
)

// Trigger defines a camera software trigger.
// When executed, it will send a 'Software' trigger signal to the camera device.
type Trigger struct {
	// Timeout is the duration for which to wait for the camera to become ready
	// to accept a trigger before sending the trigger signal.
	Timeout chrono.Duration `json:"timeout"`

	// Period is the triggering period, i.e. when Execute() is called,
	// the routine will sleep until, at least, Period time has passed after
	// the last trigger execution.
	//
	// WARNING: the period is respected loosely, exact execution timings
	// should not be expected.
	Period chrono.Duration `json:"period"`

	// WaitAfter is the wait duration after triggering.
	// It is used to allow for the camera to receive the trigger signal and
	// execute the corresponding action. Usually this is not necessary,
	// however it can be useful on slower connections,
	// eg. to avoid 'incomplete buffer read' errors.
	WaitAfter chrono.Duration `json:"wait_after"`

	// lastExecute is the time at which the trigger was last executed.
	lastExecute time.Time
}

// NewTrigger constructs a new Trigger with sensible defaults.
func NewTrigger() *Trigger {
	tout, _ := time.ParseDuration("0.1s")
	period, _ := time.ParseDuration("5s")
	return &Trigger{
		Timeout: chrono.Duration{Duration: tout},
		Period:  chrono.Duration{Duration: period},
	}
}

// Execute executes a trigger and then waits for t.TimeAfter time.
// TODO: does waiting incur penalties when used in a OS-locked routines?
// OS-locked routines shouldn't block because it reportedly causes a context
// switch which is expensive, is this also true when sleeping?
func (t *Trigger) Execute(c Camera) error {
	// wait for the period to expire
	triggerAt := t.lastExecute.Add(t.Period.Duration)
	time.Sleep(time.Until(triggerAt))

	t.lastExecute = time.Now()
	if t.Timeout.Milliseconds() == 0 {
		if !bool(C.Cam_Trigger(c.p)) {
			return errors.New("camera.Trigger.Execute: could not execute")
		}
	} else {
		if !bool(C.Cam_WaitAndTrigger(c.p, C.size_t(t.Timeout.Milliseconds()))) {
			return errors.New("camera.Trigger.Execute: could not wait and execute")
		}
	}
	time.Sleep(t.WaitAfter.Duration)
	return nil
}
