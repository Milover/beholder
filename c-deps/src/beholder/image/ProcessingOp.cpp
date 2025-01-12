// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ProcessingOp.h"

#include <algorithm>
#include <memory>
#include <opencv2/core/mat.hpp>
#include <string>
#include <utility>
#include <vector>

#include "capi/Result.h"

namespace beholder {

bool ProcessingOp::operator()(const cv::Mat& in, cv::Mat& out) const {
	return execute(in, out);
}

bool ProcessingOp::operator()(const cv::Mat& in, cv::Mat& out,
							  const std::vector<Result>& res) const {
	return execute(in, out, res);
}

}  // namespace beholder
