// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_DIV_GAUSSIAN_BLUR_H
#define BEHOLDER_IMAGE_OPS_DIV_GAUSSIAN_BLUR_H

#include <vector>

#include "beholder/image/ops/GaussianBlur.h"
#include "beholder/util/Constants.h"

namespace cv {
class Mat;
}

namespace beholder {

// A background removal operation by means of Gaussian blur division.
class DivGaussianBlur : public GaussianBlur {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// Scale factor.
	float scaleFactor{cst::max8bit};

	// Default constructor.
	DivGaussianBlur() = default;

	// NOLINTBEGIN(*-magic-numbers, bugprone-easily-swappable-parameters)

	// Default constructor.
	explicit DivGaussianBlur(float scale = cst::max8bit, float sX = 33.0,
							 float sY = 33.0, int kW = 0, int kH = 0)
		: GaussianBlur(kW, kH, sX, sY), scaleFactor{scale} {}

	// NOLINTEND(*-magic-numbers, bugprone-easily-swappable-parameters)

	DivGaussianBlur(const DivGaussianBlur&) = default;
	DivGaussianBlur(DivGaussianBlur&&) = default;

	~DivGaussianBlur() override = default;

	DivGaussianBlur& operator=(const DivGaussianBlur&) = default;
	DivGaussianBlur& operator=(DivGaussianBlur&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_DIV_GAUSSIAN_BLUR_H
