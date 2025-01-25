// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/image/ops/DivGaussianBlur.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ops/GaussianBlur.h"

namespace beholder {

bool DivGaussianBlur::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::Mat tmp{in.clone()};
	GaussianBlur::execute(in, tmp);
	cv::divide(in, tmp, out, scaleFactor);
	return true;
}

bool DivGaussianBlur::execute(
	const cv::Mat& in, cv::Mat& out,
	[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
