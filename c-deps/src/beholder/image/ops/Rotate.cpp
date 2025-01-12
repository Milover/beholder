// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/Rotate.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "capi/Result.h"
#include "image/ProcessingOp.h"
#include "util/Constants.h"

namespace beholder {

bool Rotate::execute(const cv::Mat& in, cv::Mat& out) const {
	// get rotation matrix for rotating the image
	// around it's center in pixel coordinates
	const cv::Point2f center{static_cast<float>(in.cols - 1) / 2,
							 static_cast<float>(in.rows - 1) / 2};
	cv::Mat rot{cv::getRotationMatrix2D(center, angle, 1.0)};

	// determine bounding rectangle, center not relevant
	const cv::Rect2f bbox{
		cv::RotatedRect{center, in.size(), angle}.boundingRect2f()};

	// adjust transformation matrix
	rot.at<double>(0, 2) += (static_cast<double>(bbox.width) - in.cols) / 2;
	rot.at<double>(1, 2) += (static_cast<double>(bbox.height) - in.rows) / 2;

	// assume a white background
	cv::warpAffine(in, out, rot, bbox.size(), cv::INTER_NEAREST,
				   cv::BORDER_CONSTANT,
				   cv::Scalar{cst::max8bit, cst::max8bit, cst::max8bit});
	return true;
}

bool Rotate::execute(const cv::Mat& in, cv::Mat& out,
					 [[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
