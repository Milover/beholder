// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/AdaptiveThreshold.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "capi/Result.h"
#include "image/ProcessingOp.h"
#include "util/Traits.h"

namespace beholder {

// Static checks which enforce compliance between our thresholding modes and
// the OpenCV supported types.
static_assert(enums::to(AdaptiveThreshold::Type::Mean) ==
			  enums::to(cv::ADAPTIVE_THRESH_MEAN_C));
static_assert(enums::to(AdaptiveThreshold::Type::Gaussian) ==
			  enums::to(cv::ADAPTIVE_THRESH_GAUSSIAN_C));

bool AdaptiveThreshold::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::adaptiveThreshold(in, out, maxValue, enums::to(type), cv::THRESH_BINARY,
						  size, c);
	return true;
}

bool AdaptiveThreshold::execute(
	const cv::Mat& in, cv::Mat& out,
	[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
