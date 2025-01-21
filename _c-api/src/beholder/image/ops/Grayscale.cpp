// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/image/ops/Grayscale.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"

namespace beholder {

bool Grayscale::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::cvtColor(in, out, cv::COLOR_BGR2GRAY, 1);
	return true;
}

bool Grayscale::execute(const cv::Mat& in, cv::Mat& out,
						[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
