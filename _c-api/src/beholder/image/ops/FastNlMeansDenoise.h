// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_FAST_NL_MEANS_DENOISE_H
#define BEHOLDER_IMAGE_OPS_FAST_NL_MEANS_DENOISE_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class FastNlMeansDenoise : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// Weight regulating filtering strength.
	// Bigger weights remove more noise (and image details).
	float weight{1};

	// Default constructor.
	FastNlMeansDenoise() = default;

	// Default constructor.
	explicit FastNlMeansDenoise(float w) : weight{w} {}

	FastNlMeansDenoise(const FastNlMeansDenoise&) = default;
	FastNlMeansDenoise(FastNlMeansDenoise&&) = default;

	~FastNlMeansDenoise() override = default;

	FastNlMeansDenoise& operator=(const FastNlMeansDenoise&) = default;
	FastNlMeansDenoise& operator=(FastNlMeansDenoise&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_FAST_NL_MEANS_DENOISE_H
