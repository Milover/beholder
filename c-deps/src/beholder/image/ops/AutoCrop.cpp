// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/image/ops/AutoCrop.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"

namespace beholder {

bool AutoCrop::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::RotatedRect box{};
	cv::Point2f center{};
	const bool ok{executeImpl(in, out, box, center)};
	if (!ok) {
		return false;
	}

	// crop (now centered on the image)
	cv::Rect crop{cv::RotatedRect{center, box.size, 0}.boundingRect()};
	// snap to bounds
	crop.x = crop.x > 0 ? crop.x : 0;
	crop.x = crop.x < out.cols ? crop.x : out.cols - 1;
	crop.width =
		crop.x + crop.width <= out.cols ? crop.width : out.cols - crop.x;

	crop.y = crop.y > 0 ? crop.y : 0;
	crop.y = crop.y < out.rows ? crop.y : out.rows - 1;
	crop.height =
		crop.y + crop.height <= out.rows ? crop.height : out.rows - crop.y;

	out = out(crop);
	return true;
}

bool AutoCrop::execute(const cv::Mat& in, cv::Mat& out,
					   [[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
