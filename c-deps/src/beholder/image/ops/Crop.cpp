// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/Crop.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <vector>

#include "capi/Result.h"
#include "image/ProcessingOp.h"

namespace beholder {

bool Crop::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::Rect crop{left, top, width, height};
	// snap to bounds
	crop.x = crop.x > 0 ? crop.x : 0;
	crop.x = crop.x < in.cols ? crop.x : in.cols - 1;
	crop.width = crop.x + crop.width <= in.cols ? crop.width : in.cols - crop.x;

	crop.y = crop.y > 0 ? crop.y : 0;
	crop.y = crop.y < in.rows ? crop.y : in.rows - 1;
	crop.height =
		crop.y + crop.height <= in.rows ? crop.height : in.rows - crop.y;

	out = out(crop);
	return true;
}

bool Crop::execute(const cv::Mat& in, cv::Mat& out,
				   [[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
