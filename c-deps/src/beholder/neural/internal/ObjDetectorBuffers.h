// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// A helper class for holding temporary structures created during
// object detection.

#ifndef BEHOLDER_NEURAL_INTERNAL_OBJ_DETECTOR_BUFFERS_H
#define BEHOLDER_NEURAL_INTERNAL_OBJ_DETECTOR_BUFFERS_H

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <vector>

namespace beholder {
namespace internal {

class ObjDetectorBuffers {
public:
	// Temporaries used during ObjDetector::detect and ObjDetector::extract.
	cv::Mat blob;
	std::vector<cv::Mat> outs;	// forward results
	// XXX: could also use RotatedRects
	std::vector<cv::Rect> tBoxes;	  // unfiltered blob boxes
	std::vector<double> tAngles;	  // box rotation angles
	std::vector<int> tClassIDs;		  // unfiltered class IDs
	std::vector<float> tConfidences;  // unfiltered confidences
	std::vector<int> tNMSIDs;		  // IDs used during NMS filtering

	// Clear buffers, but keep allocated memory.
	void clear() {
		outs.clear();
		tBoxes.clear();
		tAngles.clear();
		tClassIDs.clear();
		tConfidences.clear();
		tNMSIDs.clear();
	}
};

}  // namespace internal
}  // namespace beholder

#endif	// BEHOLDER_NEURAL_INTERNAL_OBJ_DETECTOR_BUFFERS_H
