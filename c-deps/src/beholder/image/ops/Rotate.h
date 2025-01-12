// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_ROTATE_H
#define BEHOLDER_IMAGE_OPS_ROTATE_H

#include <vector>

#include "image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class Rotate : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	float angle{};	// angle of rotation

	// Default constructor.
	Rotate() = default;

	// Default constructor.
	explicit Rotate(float ang) : angle{ang} {}

	Rotate(const Rotate&) = default;
	Rotate(Rotate&&) = default;

	~Rotate() override = default;

	Rotate& operator=(const Rotate&) = default;
	Rotate& operator=(Rotate&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_ROTATE_H
