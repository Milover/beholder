// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/Resize.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "capi/Result.h"
#include "image/ProcessingOp.h"

namespace beholder {

bool Resize::execute(const cv::Mat& in, cv::Mat& out) const {
	// we usually shrink images
	int interp{cv::INTER_AREA};
	if (width * height > in.rows * in.cols) {
		// when we're enlarging, usually the quality is already pretty bad
		// and the image should be pretty small so INTER_CUBIC should
		// in general be better for our use cases than INTER_LINEAR even
		// though it's slower
		interp = cv::INTER_CUBIC;
	}
	cv::resize(in, out, cv::Size(width, height), 0, 0, interp);
	return true;
}

bool Resize::execute(const cv::Mat& in, cv::Mat& out,
					 [[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
