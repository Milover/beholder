// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/image/ops/CorrectGamma.h"

#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/image/ProcessingOp.h"
#include "beholder/util/Constants.h"

namespace beholder {

bool CorrectGamma::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::Mat lut{cv::Size{1, cst::max8bit + 1}, CV_8U};
	for (auto i{0UL}; i < cst::max8bit + 1; ++i) {
		lut.at<uchar>(static_cast<int>(i)) = cv::saturate_cast<uchar>(
			std::pow(static_cast<double>(i) / cst::max8bit, gamma) *
			cst::max8bit);
	}
	cv::LUT(in, lut, out);
	return true;
}

bool CorrectGamma::execute(
	const cv::Mat& in, cv::Mat& out,
	[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
