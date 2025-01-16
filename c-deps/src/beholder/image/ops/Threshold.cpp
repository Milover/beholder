// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/image/ops/Threshold.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"
#include "beholder/util/Enums.h"

namespace beholder {

namespace {
// Static checks which enforce compliance between our thresholding modes and
// the OpenCV supported types.
using Typ = Threshold::Type;
static_assert(Typ::Binary == cv::THRESH_BINARY);
static_assert(Typ::BinaryInv == cv::THRESH_BINARY_INV);
static_assert(Typ::Truncate == cv::THRESH_TRUNC);
static_assert(Typ::ToZero == cv::THRESH_TOZERO);
static_assert(Typ::ToZeroInv == cv::THRESH_TOZERO_INV);
static_assert(Typ::Mask == cv::THRESH_MASK);
static_assert(Typ::Otsu == cv::THRESH_OTSU);
static_assert(Typ::Triangle == cv::THRESH_TRIANGLE);
}  // namespace

bool Threshold::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::threshold(in, out, threshold, maxValue, enums::to(type));
	return true;
}

bool Threshold::execute(const cv::Mat& in, cv::Mat& out,
						[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
