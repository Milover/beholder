// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_MEDIAN_BLUR_H
#define BEHOLDER_IMAGE_OPS_MEDIAN_BLUR_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class MedianBlur : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	int kernelSize{3};	// kernel size

	// Default constructor
	MedianBlur() = default;

	// Default constructor.
	explicit MedianBlur(int ksize) : kernelSize{ksize} {}

	MedianBlur(const MedianBlur&) = default;
	MedianBlur(MedianBlur&&) = default;

	~MedianBlur() override = default;

	MedianBlur& operator=(const MedianBlur&) = default;
	MedianBlur& operator=(MedianBlur&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_MEDIAN_BLUR_H
