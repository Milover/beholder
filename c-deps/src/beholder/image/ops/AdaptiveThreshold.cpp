// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/image/ops/AdaptiveThreshold.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"
#include "beholder/util/Enums.h"

namespace beholder {

namespace {
// Static checks which enforce compliance between our thresholding modes and
// the OpenCV supported types.
using Typ = AdaptiveThreshold::Type;
static_assert(Typ::Mean == cv::ADAPTIVE_THRESH_MEAN_C);
static_assert(Typ::Gaussian == cv::ADAPTIVE_THRESH_GAUSSIAN_C);
}  // namespace

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
