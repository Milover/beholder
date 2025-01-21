// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_AUTO_ORIENT_H
#define BEHOLDER_IMAGE_OPS_AUTO_ORIENT_H

#include <array>
#include <vector>

#include "beholder/image/ProcessingOp.h"
#include "beholder/util/Constants.h"

namespace cv {
class Mat;
class RotatedRect;
template<typename T>
class Point_;  // for Point2f, i.e. Point_<float>
}  // namespace cv

namespace beholder {

class AutoOrient : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

	// Execute implementation helper.
	bool executeImpl(const cv::Mat& in, cv::Mat& out, cv::RotatedRect& box,
					 cv::Point_<float>& center) const;

public:
	// NOLINTBEGIN(*-magic-numbers)

	int kernelSize{50};				// kernel size
	float textHeight{50};			// minimum recognized text box height
	float textWidth{50};			// minimum recognized text width height
	float padding{10.0};			// padding added to the cropped image
	double padValue{cst::max8bit};	// pixel value used for padding
	int gradientKernelSize{3};		// gradient kernel size

	// NOLINTEND(*-magic-numbers)

	// Default constructor.
	AutoOrient() = default;

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	AutoOrient(int kS, float tH, float tW, float pad, double padV)
		: kernelSize{kS},
		  textHeight{tH},
		  textWidth{tW},
		  padding{pad},
		  padValue{padV} {}

	AutoOrient(const AutoOrient&) = default;
	AutoOrient(AutoOrient&&) = default;

	~AutoOrient() override = default;

	AutoOrient& operator=(const AutoOrient&) = default;
	AutoOrient& operator=(AutoOrient&&) = default;
};

void findTextBox(const cv::Mat& in, int kSize, float txtHeight, float txtWidth,
				 float padding, int gKSize, cv::RotatedRect& returnValue);

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_AUTO_ORIENT_H
