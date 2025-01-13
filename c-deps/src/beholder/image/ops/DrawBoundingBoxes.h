// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_DRAW_BOUNDING_BOXES_H
#define BEHOLDER_IMAGE_OPS_DRAW_BOUNDING_BOXES_H

#include <array>
#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

// An operation for drawing bounding boxes on an image.
// Bounding box definitions are taken from a list of processing results.
class DrawBoundingBoxes : public ProcessingOp {
protected:
	// Execute the processing operation.
	// No-op, always returns true.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	using Color = std::array<float, 4>;

	Color color{0.0, 0.0, 0.0, 0.0};  // bounding box border color
	int thickness{2};				  // bounding box border thickness

	// Default constructor.
	DrawBoundingBoxes() = default;

	// Default constructor.
	DrawBoundingBoxes(const Color& c, int t) : color{c}, thickness{t} {}

	DrawBoundingBoxes(const DrawBoundingBoxes&) = default;
	DrawBoundingBoxes(DrawBoundingBoxes&&) = default;

	~DrawBoundingBoxes() override = default;

	DrawBoundingBoxes& operator=(const DrawBoundingBoxes&) = default;
	DrawBoundingBoxes& operator=(DrawBoundingBoxes&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_DRAW_BOUNDING_BOXES_H
