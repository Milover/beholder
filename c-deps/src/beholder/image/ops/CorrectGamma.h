// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_CORRECT_GAMMA_H
#define BEHOLDER_IMAGE_OPS_CORRECT_GAMMA_H

#include <vector>

#include "image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class CorrectGamma : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	double gamma{1.0};	// gamma correction value

	// Default constructor.
	CorrectGamma() = default;

	// Default constructor.
	explicit CorrectGamma(double g) : gamma{g} {}

	CorrectGamma(const CorrectGamma&) = default;
	CorrectGamma(CorrectGamma&&) = default;

	~CorrectGamma() override = default;

	CorrectGamma& operator=(const CorrectGamma&) = default;
	CorrectGamma& operator=(CorrectGamma&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_CORRECT_GAMMA_H
