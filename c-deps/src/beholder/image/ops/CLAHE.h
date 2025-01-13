// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_CLAHE_H
#define BEHOLDER_IMAGE_OPS_CLAHE_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

// A contrast limited adaptive histogram equalization operation.
class CLAHE : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// NOLINTBEGIN(*-magic-numbers)

	float clipLimit{40.0};	// clip percentage
	int tileRows{8};		// number of tile rows
	int tileColumns{8};		// number of tile columns

	// NOLINTEND(*-magic-numbers)

	// Default constructor.
	CLAHE() = default;

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	CLAHE(float cLim, int tR, int tC)
		: clipLimit{cLim}, tileRows{tR}, tileColumns{tC} {}

	CLAHE(const CLAHE&) = default;
	CLAHE(CLAHE&&) = default;

	~CLAHE() override = default;

	CLAHE& operator=(const CLAHE&) = default;
	CLAHE& operator=(CLAHE&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_CLAHE_H
