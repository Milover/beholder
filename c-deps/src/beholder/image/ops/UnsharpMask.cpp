// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/UnsharpMask.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "capi/Result.h"

namespace beholder {

bool UnsharpMask::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::Mat blurred{};
	cv::GaussianBlur(in, blurred, cv::Size{}, sigma, sigma);
	const cv::Mat lowContrastMask{cv::abs(in - blurred) < threshold};
	cv::Mat sharp{in * (1.0 + amount) + blurred * (-amount)};
	in.copyTo(sharp, lowContrastMask);
	out = sharp;
	return true;
}

bool UnsharpMask::execute(
	const cv::Mat& in, cv::Mat& out,
	[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
