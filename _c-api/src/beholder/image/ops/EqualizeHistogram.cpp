// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/image/ops/EqualizeHistogram.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace beholder {

bool EqualizeHistogram::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::equalizeHist(in, out);
	return true;
}

bool EqualizeHistogram::execute(
	const cv::Mat& in, cv::Mat& out,
	[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
