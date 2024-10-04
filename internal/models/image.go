package models

import (
	"fmt"
	"time"
	"unsafe"
)

// An Image is a (non-owning) view of a camera acquisition result.
//
// TODO: a Source tag would be nice, so we know where the image came from,
// eg. a camera S/N, or a file name if the image was read from disc.
type Image struct {
	// ID is the acquisition result id as asigned by the camera.
	//
	// TODO: change to UUIDv7 which we assign, fuck the camera.
	ID uint64
	// Timestamp is the time at which the image was created or acquired, i.e.
	// received by the host machine.
	//
	// TODO: remove when ID gets changed to UUID, because UUIDv7
	// embeds a timestamp.
	Timestamp time.Time
	// Buffer is a non-owning (weak) pointer to the raw bytes of
	// an un-encoded image.
	Buffer unsafe.Pointer

	Rows         int    // height of the image in pixels.
	Cols         int    // width of the image in pixels.
	PixelType    int64  // pixel type of the image
	Step         uint64 // number of bits per image row
	BitsPerPixel uint64 // number of bits per pixel
}

// String returns the string representation of img.
func (img Image) String() string {
	return fmt.Sprintf(`id: %v
timestamp: %v
rows: %v
cols: %v
step: %v
bits per pixel: %v
pixel type: %v
buffer: %v`,
		img.ID,
		img.Timestamp,
		img.Rows,
		img.Cols,
		img.Step,
		img.BitsPerPixel,
		img.PixelType,
		img.Buffer,
	)
}
