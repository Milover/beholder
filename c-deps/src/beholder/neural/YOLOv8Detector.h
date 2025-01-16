// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// An object detector using the YOLOv8 model.
// A DNN-based object detector wrapper using the YOLOv8 model.
// For more info, see: https://github.com/ultralytics/ultralytics

#ifndef BEHOLDER_NEURAL_YOLOV8_DETECTOR_H
#define BEHOLDER_NEURAL_YOLOV8_DETECTOR_H

#include "beholder/neural/ObjDetector.h"

namespace beholder {

class YOLOv8Detector : public ObjDetector {
public:
	using Base = ObjDetector;

protected:
	// Extract inference results.
	void extract() override;

	// Filter (NMS) and store results.
	void store() override;

public:
	// Default constructor.
	YOLOv8Detector();

	YOLOv8Detector(const YOLOv8Detector&) = delete;
	YOLOv8Detector(YOLOv8Detector&&) = default;

	~YOLOv8Detector() override = default;

	YOLOv8Detector& operator=(const YOLOv8Detector&) = delete;
	YOLOv8Detector& operator=(YOLOv8Detector&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_NEURAL_YOLOV8_DETECTOR_H
