// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_CROP_H
#define BEHOLDER_IMAGE_OPS_CROP_H

#include <vector>

#include "image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class Crop : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// Crop boundaries (inclusive)
	int left{};
	int top{};
	int width{};
	int height{};

	// Default constructor.
	Crop() = default;

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	Crop(int l, int t, int w, int h) : left{l}, top{t}, width{w}, height{h} {}

	Crop(const Crop&) = default;
	Crop(Crop&&) = default;

	~Crop() override = default;

	Crop& operator=(const Crop&) = default;
	Crop& operator=(Crop&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_CROP_H
