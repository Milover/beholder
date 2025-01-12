// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_AUTO_CROP_H
#define BEHOLDER_IMAGE_OPS_AUTO_CROP_H

#include <vector>

#include "image/ops/AutoOrient.h"

namespace cv {
class Mat;
}

namespace beholder {

class AutoCrop : public AutoOrient {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// Default constructor.
	AutoCrop() = default;

	// Default constructor.
	AutoCrop(int kS, float tH, float tW, float pad, double padV)
		: AutoOrient(kS, tH, tW, pad, padV) {}

	AutoCrop(const AutoCrop&) = default;
	AutoCrop(AutoCrop&&) = default;

	~AutoCrop() override = default;

	AutoCrop& operator=(const AutoCrop&) = default;
	AutoCrop& operator=(AutoCrop&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_AUTO_CROP_H
