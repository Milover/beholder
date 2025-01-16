// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_THRESHOLD_H
#define BEHOLDER_IMAGE_OPS_THRESHOLD_H

#include <vector>

#include "beholder/image/ProcessingOp.h"
#include "beholder/util/Constants.h"
#include "beholder/util/Enums.h"

namespace cv {
class Mat;
}

namespace beholder {

class Threshold : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// Supported types of thresholding. For more info, see:
	// https://docs.opencv.org/4.10.0/d7/d1b/group__imgproc__misc.html#gaa9e58d2860d4afa658ef70a9b1115576
	enum class Type {
		Binary,
		BinaryInv,
		Truncate,
		ToZero,
		ToZeroInv,
		Mask = 7,
		Otsu,
		Triangle = 16
	};

	float threshold{0.0};				   // threshold value
	float maxValue{cst::max8bit};		   // max threshold value
	Type type{Type::Binary + Type::Otsu};  // threshold type

	// Default constructor.
	Threshold() = default;

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	Threshold(float thresh, float maxv, Type typ)
		: threshold{thresh}, maxValue{maxv}, type{typ} {}

	Threshold(const Threshold&) = default;
	Threshold(Threshold&&) = default;

	~Threshold() override = default;

	Threshold& operator=(const Threshold&) = default;
	Threshold& operator=(Threshold&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_THRESHOLD_H
