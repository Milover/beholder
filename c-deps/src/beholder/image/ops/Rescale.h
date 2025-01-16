// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_RESCALE_H
#define BEHOLDER_IMAGE_OPS_RESCALE_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class Rescale : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	double scale{};	 // scale factor

	// Default constructor.
	Rescale() = default;

	// Default constructor.
	explicit Rescale(double s) : scale{s} {}

	Rescale(const Rescale&) = default;
	Rescale(Rescale&&) = default;

	~Rescale() override = default;

	Rescale& operator=(const Rescale&) = default;
	Rescale& operator=(Rescale&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_RESCALE_H
