// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/image/ops/BGR.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"

namespace beholder {

bool BGR::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::cvtColor(in, out, cv::COLOR_GRAY2BGR, 3);
	return true;
}

bool BGR::execute(const cv::Mat& in, cv::Mat& out,
				  [[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
