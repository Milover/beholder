// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_DRAW_LABELS_H
#define BEHOLDER_IMAGE_OPS_DRAW_LABELS_H

#include <array>
#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

// An operation for drawing text labels on an image.
// Labels are drawn based on processing results.
class DrawLabels : public ProcessingOp {
protected:
	// Execute the processing operation.
	// No-op, always returns true.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	using Color = std::array<float, 4>;

	Color color{0.0, 0.0, 0.0, 0.0};  // font color
	double fontScale{1};			  // font scale
	int thickness{2};				  // line thickness

	// Default constructor.
	DrawLabels() = default;

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	DrawLabels(const Color& c, double s, int t)
		: color{c}, fontScale{s}, thickness{t} {}

	DrawLabels(const DrawLabels&) = default;
	DrawLabels(DrawLabels&&) = default;

	~DrawLabels() override = default;

	DrawLabels& operator=(const DrawLabels&) = default;
	DrawLabels& operator=(DrawLabels&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_DRAW_LABELS_H
