// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/NormalizeBrightnessContrast.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "image/ProcessingOp.h"
#include "util/Constants.h"

namespace beholder {

bool NormalizeBrightnessContrast::execute(const cv::Mat& in,
										  cv::Mat& out) const {
	// compute histogram
	const std::vector<cv::Mat> input{in};  // FIXME: this is wasteful
	const std::vector<int> channels{0};	   // FIXME: read from image
	std::vector<int> histSize{cst::max8bit};
	cv::Mat hist;

	cv::calcHist(input, channels, cv::Mat{}, hist, histSize,
				 std::vector<float>{});
	// compute cumulative distribution
	std::vector<float> acc;
	acc.reserve(hist.total());
	acc.emplace_back(hist.at<float>(0));
	for (auto i{1UL}; i < hist.total(); ++i) {
		acc.emplace_back(acc[i - 1] + hist.at<float>(static_cast<int>(i)));
	}
	// locate clip points
	const float max{acc.back()};
	const float lo{clipLowPct * max / 100.0F};	 // percent to value
	const float hi{clipHighPct * max / 100.0F};	 // percent to value
	// FIXME: this is looks kinda dumb
	// locate left cut
	int min_gray{0};
	while (acc[min_gray] < lo) {
		++min_gray;
	}
	// locate right cut
	int max_gray{histSize[0] - 1};
	while (acc[max_gray] >= (max - hi)) {
		--max_gray;
	}
	const float alpha{static_cast<float>(cst::max8bit) /
					  static_cast<float>((max_gray - min_gray))};
	const float beta{static_cast<float>(-min_gray) * alpha};

	cv::convertScaleAbs(in, out, alpha, beta);

	return true;
}

bool NormalizeBrightnessContrast::execute(
	const cv::Mat& in, cv::Mat& out,
	[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
