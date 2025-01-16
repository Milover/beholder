// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_BGR_H
#define BEHOLDER_IMAGE_OPS_BGR_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

// A grayscale-image-to-image conversion operation.
class BGR : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// Default constructor.
	BGR() = default;

	BGR(const BGR&) = default;
	BGR(BGR&&) = default;

	~BGR() override = default;

	BGR& operator=(const BGR&) = default;
	BGR& operator=(BGR&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_BGR_H
