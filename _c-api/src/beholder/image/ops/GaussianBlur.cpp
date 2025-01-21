// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/image/ops/GaussianBlur.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"

namespace beholder {

bool GaussianBlur::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::GaussianBlur(in, out, cv::Size(kernelWidth, kernelHeight), sigmaX,
					 sigmaY);
	return true;
}

bool GaussianBlur::execute(
	const cv::Mat& in, cv::Mat& out,
	[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
