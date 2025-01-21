// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_DEBLUR_H
#define BEHOLDER_IMAGE_OPS_DEBLUR_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
template<typename T>
class Size_;  // for Size == Size2i == Size_<int>
}  // namespace cv

namespace beholder {

// An out of focus image deblurring operation.
//
// For more info, see:
// https://docs.opencv.org/4.10.0/de/d3c/tutorial_out_of_focus_deblur_filter.html
class Deblur : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// NOLINTBEGIN(*-magic-numbers)

	int radius{5};	// deblur radius
	int snr{100};	// signal to noise ratio

	// NOLINTEND(*-magic-numbers)

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	Deblur(int rad, int snrRatio) : radius{rad}, snr{snrRatio} {}

	// Default constructor.
	Deblur() = default;

	Deblur(const Deblur&) = default;
	Deblur(Deblur&&) = default;

	~Deblur() override = default;

	Deblur& operator=(const Deblur&) = default;
	Deblur& operator=(Deblur&&) = default;
};

void computePSF(cv::Mat& out, const cv::Size_<int>& filterSize, int R);

void fftShift(const cv::Mat& in, cv::Mat& out);

void filter2Dfreq(const cv::Mat& in, cv::Mat& out, const cv::Mat& H);

void computeWeinerFilter(const cv::Mat& in, cv::Mat& out, double nsr);

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_DEBLUR_H
