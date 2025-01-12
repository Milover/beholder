// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/CLAHE.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "image/ProcessingOp.h"

namespace beholder {

bool CLAHE::execute(const cv::Mat& in, cv::Mat& out) const {
	const cv::Ptr<cv::CLAHE> clahe{
		cv::createCLAHE(clipLimit, cv::Size{tileRows, tileColumns})};
	clahe->apply(in, out);
	return true;
}

bool CLAHE::execute(const cv::Mat& in, cv::Mat& out,
					[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
