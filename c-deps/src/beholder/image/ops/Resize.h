// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_RESIZE_H
#define BEHOLDER_IMAGE_OPS_RESIZE_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class Resize : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// New image dimensions.
	int width{};
	int height{};

	// Default constructor.
	Resize() = default;

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	Resize(int w, int h) : width{w}, height{h} {}

	Resize(const Resize&) = default;
	Resize(Resize&&) = default;

	~Resize() override = default;

	Resize& operator=(const Resize&) = default;
	Resize& operator=(Resize&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_RESIZE_H
