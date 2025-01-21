// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_NORMALIZE_BRIGHTNESS_CONTRAST_H
#define BEHOLDER_IMAGE_OPS_NORMALIZE_BRIGHTNESS_CONTRAST_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

// Normalize image brightness and contrast.
//
// Taken from: https://stackoverflow.com/a/56909036/17881968
class NormalizeBrightnessContrast : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// NOLINTBEGIN(*-magic-numbers)

	float clipLowPct{0.25};	  // low-value side clip percentage
	float clipHighPct{0.25};  // high-value side percentage

	// NOLINTEND(*-magic-numbers)

	// Default constructor.
	NormalizeBrightnessContrast() = default;

	// Default constructor.
	// Clip 'cPct' of image values in total, symmetrically from both sides
	explicit NormalizeBrightnessContrast(float cPct)
		: clipLowPct{cPct / 2}, clipHighPct{cPct / 2} {}

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	NormalizeBrightnessContrast(float cLowPct, float cHighPct)
		: clipLowPct{cLowPct}, clipHighPct{cHighPct} {}

	NormalizeBrightnessContrast(const NormalizeBrightnessContrast&) = default;
	NormalizeBrightnessContrast(NormalizeBrightnessContrast&&) = default;

	~NormalizeBrightnessContrast() override = default;

	NormalizeBrightnessContrast&
	operator=(const NormalizeBrightnessContrast&) = default;
	NormalizeBrightnessContrast&
	operator=(NormalizeBrightnessContrast&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_NORMALIZE_BRIGHTNESS_CONTRAST_H
