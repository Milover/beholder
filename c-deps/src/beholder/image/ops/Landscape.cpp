// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/Landscape.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>

#include "capi/Result.h"
#include "image/ProcessingOp.h"

namespace beholder {

bool Landscape::execute(const cv::Mat& in, cv::Mat& out) const {
	if (in.cols < in.rows) {
		cv::rotate(in, out, cv::ROTATE_90_CLOCKWISE);
	}
	return true;
}

bool Landscape::execute(const cv::Mat& in, cv::Mat& out,
						[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
