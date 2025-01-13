// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_UNSHARP_MASK_H
#define BEHOLDER_IMAGE_OPS_UNSHARP_MASK_H

#include <vector>

#include "beholder/image/ops/GaussianBlur.h"

namespace cv {
class Mat;
}

namespace beholder {

// An image sharpennig operation using the 'unsharp mask' algorithm.
//
// For more info, see:
// https://docs.opencv.org/4.10.0/d1/d10/classcv_1_1MatExpr.html#details
class UnsharpMask : public GaussianBlur {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// NOLINTBEGIN(*-magic-numbers)

	double sigma{1.0};		// Gaussian kernel standard deviation
	double threshold{5.0};	// low-contrast mask threshold value
	double amount{1.0};		// ammount of sharpenning to apply

	// NOLINTEND(*-magic-numbers)

	// Default constructor
	UnsharpMask() = default;

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	UnsharpMask(double s, double t, double a)
		: sigma{s}, threshold{t}, amount{a} {}

	UnsharpMask(const UnsharpMask&) = default;
	UnsharpMask(UnsharpMask&&) = default;

	~UnsharpMask() override = default;

	UnsharpMask& operator=(const UnsharpMask&) = default;
	UnsharpMask& operator=(UnsharpMask&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_UNSHARP_MASK_H
