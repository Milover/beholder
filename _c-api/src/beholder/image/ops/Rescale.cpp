// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/image/ops/Rescale.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"

namespace beholder {

bool Rescale::execute(const cv::Mat& in, cv::Mat& out) const {
	// we usually shrink images
	int interp{cv::INTER_AREA};
	if (scale > 1.0) {
		// when we're enlarging, usually the quality is already pretty bad
		// and the image should be pretty small so INTER_CUBIC should
		// in general be better for our use cases than INTER_LINEAR even
		// though it's slower
		interp = cv::INTER_CUBIC;
	}
	cv::resize(in, out, cv::Size{}, scale, scale, interp);
	return true;
}

bool Rescale::execute(const cv::Mat& in, cv::Mat& out,
					  [[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
