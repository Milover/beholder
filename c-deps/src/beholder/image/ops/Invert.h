// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_INVERT_H
#define BEHOLDER_IMAGE_OPS_INVERT_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class Invert : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	Invert() = default;
	Invert(const Invert&) = default;
	Invert(Invert&&) = default;

	~Invert() override = default;

	Invert& operator=(const Invert&) = default;
	Invert& operator=(Invert&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_INVERT_H
