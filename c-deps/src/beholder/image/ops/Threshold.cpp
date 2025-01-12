// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/Threshold.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "capi/Result.h"
#include "image/ProcessingOp.h"
#include "util/Traits.h"

namespace beholder {

// Static checks which enforce compliance between our thresholding modes and
// the OpenCV supported types.
using Typ = Threshold::Type;
static_assert(enums::to(Typ::Binary) == enums::to(cv::THRESH_BINARY));
static_assert(enums::to(Typ::BinaryInv) == enums::to(cv::THRESH_BINARY_INV));
static_assert(enums::to(Typ::Truncate) == enums::to(cv::THRESH_TRUNC));
static_assert(enums::to(Typ::ToZero) == enums::to(cv::THRESH_TOZERO));
static_assert(enums::to(Typ::ToZeroInv) == enums::to(cv::THRESH_TOZERO_INV));
static_assert(enums::to(Typ::Mask) == enums::to(cv::THRESH_MASK));
static_assert(enums::to(Typ::Otsu) == enums::to(cv::THRESH_OTSU));
static_assert(enums::to(Typ::Triangle) == enums::to(cv::THRESH_TRIANGLE));

bool Threshold::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::threshold(in, out, threshold, maxValue, enums::to(type));
	return true;
}

bool Threshold::execute(const cv::Mat& in, cv::Mat& out,
						[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

}  // namespace beholder
