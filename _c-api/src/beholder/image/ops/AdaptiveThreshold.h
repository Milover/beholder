// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_ADAPTIVE_THRESHOLD_H
#define BEHOLDER_IMAGE_OPS_ADAPTIVE_THRESHOLD_H

#include <vector>

#include "beholder/image/ProcessingOp.h"
#include "beholder/util/Constants.h"

namespace cv {
class Mat;
}

namespace beholder {

class AdaptiveThreshold : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// Supported thresholding modes.
	enum class Type { Mean, Gaussian };

	// NOLINTBEGIN(*-magic-numbers)

	double maxValue{cst::max8bit};	// max value
	int size{11};					// kernel size
	double c{2};   // constant subtracted from the mean or weighted mean
	Type type{1};  // thresholding mode

	// NOLINTEND(*-magic-numbers)

	// Default constructor.
	AdaptiveThreshold() = default;

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	AdaptiveThreshold(double max, int sz, double cnst, Type typ)
		: maxValue{max}, size{sz}, c{cnst}, type{typ} {}

	AdaptiveThreshold(const AdaptiveThreshold&) = default;
	AdaptiveThreshold(AdaptiveThreshold&&) = default;

	~AdaptiveThreshold() override = default;

	AdaptiveThreshold& operator=(const AdaptiveThreshold&) = default;
	AdaptiveThreshold& operator=(AdaptiveThreshold&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_ADAPTIVE_THRESHOLD_H
