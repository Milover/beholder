// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_GRAYSCALE_H
#define BEHOLDER_IMAGE_OPS_GRAYSCALE_H

#include <vector>

#include "image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class Grayscale : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	Grayscale() = default;
	Grayscale(const Grayscale&) = default;
	Grayscale(Grayscale&&) = default;

	~Grayscale() override = default;

	Grayscale& operator=(const Grayscale&) = default;
	Grayscale& operator=(Grayscale&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_GRAYSCALE_H
