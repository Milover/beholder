// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/image/ops/Morphology.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"
#include "beholder/util/Enums.h"

namespace beholder {

namespace {
// Static checks which enforce compliance between our morphological operation
// types and the OpenCV supported ones.
using Typ = Morphology::Type;
static_assert(Typ::Erode == cv::MORPH_ERODE);
static_assert(Typ::Dilate == cv::MORPH_DILATE);
static_assert(Typ::Open == cv::MORPH_OPEN);
static_assert(Typ::Close == cv::MORPH_CLOSE);
static_assert(Typ::Gradient == cv::MORPH_GRADIENT);
static_assert(Typ::TopHat == cv::MORPH_TOPHAT);
static_assert(Typ::BlackHat == cv::MORPH_BLACKHAT);
static_assert(Typ::HitMiss == cv::MORPH_HITMISS);

// Static checks which enforce compliance between our morphology kernel shapes
// and the OpenCV supported ones.
using Shp = Morphology::Shape;
static_assert(Shp::Box == cv::MORPH_RECT);
static_assert(Shp::Cross == cv::MORPH_CROSS);
static_assert(Shp::Ellipse == cv::MORPH_ELLIPSE);
}  // namespace

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
