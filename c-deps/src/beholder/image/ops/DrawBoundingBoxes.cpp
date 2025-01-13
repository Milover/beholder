// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/image/ops/DrawBoundingBoxes.h"

#include <array>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"

namespace beholder {

bool DrawBoundingBoxes::execute([[maybe_unused]] const cv::Mat& in,
								[[maybe_unused]] cv::Mat& out) const {
	// can't really do anything without the results
	return true;
}

bool DrawBoundingBoxes::execute([[maybe_unused]] const cv::Mat& in,
								cv::Mat& out,
								const std::vector<Result>& res) const {
	const cv::Scalar c{color[0], color[1], color[2], color[3]};
	cv::RotatedRect rect{};
	//cv::Point2f verts[4]{};
	std::array<cv::Point2f, 4> verts{};
	for (const auto& r : res) {
		const auto& b{r.box.cRef()};
		rect = cv::RotatedRect{
			cv::Point2f{static_cast<float>(b.right + b.left) / 2,
						static_cast<float>(b.bottom + b.top) / 2},
			cv::Size2f{static_cast<float>(b.right - b.left),
					   static_cast<float>(b.bottom - b.top)},
			static_cast<float>(r.boxRotAngle)};
		rect.points(verts.data());
		for (auto i{0UL}; i < verts.size(); ++i) {
			cv::line(out, verts[i], verts[(i + 1) % verts.size()], c,  // NOLINT
					 thickness);
		}
	}
	return true;
}

}  // namespace beholder
