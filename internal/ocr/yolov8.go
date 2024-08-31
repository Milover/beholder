package ocr

/*
#include "ocr.h"
*/
import "C"

func init() {
	RegisterNetwork(TypeYOLOv8, "yolov8", func() Network { return NewYOLOv8() })
}

const TypeYOLOv8 Type = 2 // YOLOv8 object detection models

// YOLOv8 is an object detection [Network] using the [YOLOv8] model.
//
// The [Network] interface is implemented by the embedded [network], so the
// same usage rules apply.
//
// WARNING: YOLOv8 contains C-managed resources so when it is no longer needed,
// [YOLOv8.Delete] must be called to release the resources and clean up.
//
// [YOLOv8]: https://docs.ultralytics.com/models/yolov8/
type YOLOv8 struct {
	network // the underlying network
}

// NewYOLOv8 constructs (C call) a new YOLOv8 object detection network.
// WARNING: [YOLOv8.Delete] must be called to release the memory
// when no longer needed.
func NewYOLOv8() *YOLOv8 {
	y := &YOLOv8{network: newNetwork()}
	y.typ = TypeYOLOv8
	y.p = C.Det_NewYOLOv8()
	// YOLOv8 expects pixel values to be [0, 1], see:
	// https://docs.ultralytics.com/guides/preprocessing_annotated_data/#normalizing-pixel-values
	y.Config.Scale = 1.0 / 255.0
	return y
}
