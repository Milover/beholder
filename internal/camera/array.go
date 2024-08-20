//go:build linux

package camera

import "errors"

// Array represents an array of camera devices.
//
// Array supports most [Camera] functions, and calls them for each camera
// in the array sequentially, hence it can be used as if though it were
// a single [Camera].
type Array []*Camera

// Apply is a function which sequentially applies fn
// to each camera in the array.
func (a Array) Apply(fn func(*Camera) error) error {
	var err error
	for _, cam := range a {
		e := fn(cam)
		err = errors.Join(err, e)
	}
	return err
}

// Acquire attempts to acquire images from each camera in the array.
// See [Camera.Acquire] for more details.
//
// FIXME: this is a provisional implementation. We should not assume that
// all cameras will yield an image at the same time, instead, we should
// just acquire the first available image from any camera in the array and
// leave concurrency, thread locking and other details up to the caller.
func (a Array) Acquire() error {
	return a.Apply(func(c *Camera) error {
		return c.Acquire()
	})
}

// Delete releases the C-allocated memory of each camera in the array,
// see [Camera.Delete] for more details.
func (a Array) Delete() {
	a.Apply(func(c *Camera) error {
		c.Delete()
		return nil
	})
}

// Init initializes (C call) all camera device with configuration data,
// see [Camera.Init] for more details.
func (a Array) Init() error {
	return a.Apply(func(c *Camera) error {
		return c.Init()
	})
}

// IsAcquiring reports the image acquisition status of the camera array.
// It returns true if [Camera.IsAcquiring] returns true for all cameras
// in the array.
func (a Array) IsAcquiring() bool {
	acq := true
	for _, cam := range a {
		acq = acq && cam.IsAcquiring()
	}
	return acq
}

// StartAcquisition starts image acquisition for each camera in the array,
// see [Camera.StartAcquisition] for more details.
func (a Array) StartAcquisition() error {
	return a.Apply(func(c *Camera) error {
		return c.StartAcquisition()
	})
}

// StopAcquisition stops image acquisition for each camera in the array,
// see [Camera.StopAcquisition] for more details.
func (a Array) StopAcquisition() {
	a.Apply(func(c *Camera) error {
		c.StopAcquisition()
		return nil
	})
}

// TryTrigger is a function that will try to execute a software trigger
// for each camera in the array, see [Camera.TryTrigger] for more details.
func (a Array) TryTrigger() error {
	return a.Apply(func(c *Camera) error {
		return c.TryTrigger()
	})
}
