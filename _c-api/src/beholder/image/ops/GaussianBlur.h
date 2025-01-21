// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_GAUSSIAN_BLUR_H
#define BEHOLDER_IMAGE_OPS_GAUSSIAN_BLUR_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class GaussianBlur : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	int kernelWidth{3};	  // filter kernel width
	int kernelHeight{3};  // filter kernel width
	float sigmaX{0};	  // x-axis kernel standard deviation
	float sigmaY{0};	  // y-axis kernel width

	// Default constructor.
	GaussianBlur() = default;

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	GaussianBlur(int kW, int kH, float sX, float sY)
		: kernelWidth{kW}, kernelHeight{kH}, sigmaX{sX}, sigmaY{sY} {}

	GaussianBlur(const GaussianBlur&) = default;
	GaussianBlur(GaussianBlur&&) = default;

	~GaussianBlur() override = default;

	GaussianBlur& operator=(const GaussianBlur&) = default;
	GaussianBlur& operator=(GaussianBlur&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_GAUSSIAN_BLUR_H
