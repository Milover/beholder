// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_EQUALIZE_HISTOGRAM_H
#define BEHOLDER_IMAGE_OPS_EQUALIZE_HISTOGRAM_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class EqualizeHistogram : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	EqualizeHistogram() = default;
	EqualizeHistogram(const EqualizeHistogram&) = default;
	EqualizeHistogram(EqualizeHistogram&&) = default;

	~EqualizeHistogram() override = default;

	EqualizeHistogram& operator=(const EqualizeHistogram&) = default;
	EqualizeHistogram& operator=(EqualizeHistogram&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_EQUALIZE_HISTOGRAM_H
