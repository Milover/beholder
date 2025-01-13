// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/image/ops/AddPadding.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"

namespace beholder {

bool AddPadding::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::Mat tmp{in.rows + 2 * padding, in.cols + 2 * padding, in.depth()};
	// assume white background
	cv::copyMakeBorder(in, tmp, padding, padding, padding, padding,
					   cv::BORDER_ISOLATED, cv::Scalar::all(padValue));
	out = tmp;
	return true;
}

bool AddPadding::execute(
	const cv::Mat& in, cv::Mat& out,
	[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
