// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/image/ops/FastNlMeansDenoise.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/photo.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"

namespace beholder {

bool FastNlMeansDenoise::execute(const cv::Mat& in, cv::Mat& out) const {
	const std::vector<float> h{1, weight};
	cv::fastNlMeansDenoising(in, out, h);
	return true;
}

bool FastNlMeansDenoise::execute(
	const cv::Mat& in, cv::Mat& out,
	[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
