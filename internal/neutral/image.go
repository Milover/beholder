// Package neutral defines data types used by other packages to exchange data
// without imposing C dependencies.
package neutral

import (
	"time"
	"unsafe"
)

// An Image is a (non-owning) view of a camera acquisition result.
type Image struct {
	// ID is the acquisition result id as asigned by the camera.
	ID uint64
	// Timestamp is the time at which the image was created or acquired, i.e.
	// received by the host machine.
	Timestamp time.Time
	// Buffer is a non-owning (weak) pointer to the raw bytes of an image.
	Buffer unsafe.Pointer

	Rows         int    // height of the image in pixels.
	Cols         int    // width of the image in pixels.
	PixelType    int64  // pixel type of the image
	Step         uint64 // number of bits per image row
	BitsPerPixel uint64 // number of bits per pixel
}
