// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_RESIZE_TO_HEIGHT_H
#define BEHOLDER_IMAGE_OPS_RESIZE_TO_HEIGHT_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class ResizeToHeight : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	int height{};  // new image height

	// Default constructor.
	ResizeToHeight() = default;

	// Default constructor.
	explicit ResizeToHeight(int h) : height{h} {}

	ResizeToHeight(const ResizeToHeight&) = default;
	ResizeToHeight(ResizeToHeight&&) = default;

	~ResizeToHeight() override = default;

	ResizeToHeight& operator=(const ResizeToHeight&) = default;
	ResizeToHeight& operator=(ResizeToHeight&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_RESIZE_TO_HEIGHT_H
