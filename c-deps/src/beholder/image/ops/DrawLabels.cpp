// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/image/ops/DrawLabels.h"

#include <array>
#include <iomanip>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <sstream>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"

namespace beholder {

bool DrawLabels::execute([[maybe_unused]] const cv::Mat& in,
						 [[maybe_unused]] cv::Mat& out) const {
	// can't really do anything without the BEHOLDER results
	return true;
}

bool DrawLabels::execute([[maybe_unused]] const cv::Mat& in, cv::Mat& out,
						 const std::vector<Result>& res) const {
	const cv::Scalar c{color[0], color[1], color[2], color[3]};
	std::string label;
	for (const auto& r : res) {
		std::stringstream ss;
		ss << r.text << ": " << std::fixed << std::setprecision(2)
		   << r.confidence;
		label = ss.str();

		int baseline{};
		const cv::Size labelSize{cv::getTextSize(
			label, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline)};
		const int top{cv::max(r.box.cRef().top, labelSize.height)};
		cv::putText(out, label, cv::Point(r.box.cRef().left, top - baseline),
					cv::FONT_HERSHEY_SIMPLEX, fontScale, c, thickness);
	}
	return true;
}

}  // namespace beholder
