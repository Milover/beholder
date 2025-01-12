// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/Morphology.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "capi/Result.h"
#include "image/ProcessingOp.h"
#include "util/Traits.h"

namespace beholder {

// Static checks which enforce compliance between our morphological operation
// types and the OpenCV supported ones.
using Typ = Morphology::Type;
static_assert(enums::to(Typ::Erode) == enums::to(cv::MORPH_ERODE));
static_assert(enums::to(Typ::Dilate) == enums::to(cv::MORPH_DILATE));
static_assert(enums::to(Typ::Open) == enums::to(cv::MORPH_OPEN));
static_assert(enums::to(Typ::Close) == enums::to(cv::MORPH_CLOSE));
static_assert(enums::to(Typ::Gradient) == enums::to(cv::MORPH_GRADIENT));
static_assert(enums::to(Typ::TopHat) == enums::to(cv::MORPH_TOPHAT));
static_assert(enums::to(Typ::BlackHat) == enums::to(cv::MORPH_BLACKHAT));
static_assert(enums::to(Typ::HitMiss) == enums::to(cv::MORPH_HITMISS));

// Static checks which enforce compliance between our morphology kernel shapes
// and the OpenCV supported ones.
using Shp = Morphology::Shape;
static_assert(enums::to(Shp::Box) == enums::to(cv::MORPH_RECT));
static_assert(enums::to(Shp::Cross) == enums::to(cv::MORPH_CROSS));
static_assert(enums::to(Shp::Ellipse) == enums::to(cv::MORPH_ELLIPSE));

bool Morphology::execute(const cv::Mat& in, cv::Mat& out) const {
	const cv::Mat el =
		cv::getStructuringElement(enums::to(shape), cv::Size(width, height));
	cv::morphologyEx(in, out, enums::to(type), el, cv::Point(-1, -1),
					 iterations);
	return true;
}

bool Morphology::execute(
	const cv::Mat& in, cv::Mat& out,
	[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
