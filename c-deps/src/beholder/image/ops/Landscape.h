// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_LANDSCAPE_H
#define BEHOLDER_IMAGE_OPS_LANDSCAPE_H

#include <vector>

#include "image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

// An image operation which, optionally, rotates the image by 90° clockwise,
// so that image width is greater than image height.
// If image width is already greater than image height, the operation does
// nothing.
class Landscape : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	Landscape() = default;
	Landscape(const Landscape&) = default;
	Landscape(Landscape&&) = default;

	~Landscape() override = default;

	Landscape& operator=(const Landscape&) = default;
	Landscape& operator=(Landscape&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_LANDSCAPE_H
