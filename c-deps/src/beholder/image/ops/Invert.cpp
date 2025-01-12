// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/Invert.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>

#include "capi/Result.h"
#include "image/ProcessingOp.h"

namespace beholder {

bool Invert::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::bitwise_not(in, out);
	return true;
}

bool Invert::execute(const cv::Mat& in, cv::Mat& out,
					 [[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
