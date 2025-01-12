// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "image/ops/Deblur.h"

#include <array>
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "capi/Result.h"
#include "image/ProcessingOp.h"
#include "util/Constants.h"

namespace beholder {

bool Deblur::execute(const cv::Mat& in, cv::Mat& out) const {
	// NOTE: the tutorial uses grayscale images only?
	// even images only
	const cv::Rect roi{0, 0, in.cols & -2, in.rows & -2};

	// compute Hw
	cv::Mat Hw{};
	cv::Mat h{};
	computePSF(h, roi.size(), radius);
	computeWeinerFilter(h, Hw, 1.0 / static_cast<double>(snr));

	// filter
	filter2Dfreq(in(roi), out, Hw);

	out.convertTo(out, CV_8U);
	cv::normalize(out, out, 0, cst::max8bit, cv::NORM_MINMAX);

	return true;
}

bool Deblur::execute(const cv::Mat& in, cv::Mat& out,
					 [[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

void computePSF(cv::Mat& out, const cv::Size& filterSize, int R) {
	cv::Mat h{filterSize, CV_32F, cv::Scalar::all(0)};
	const cv::Point point{filterSize.width / 2, filterSize.height / 2};
	cv::circle(h, point, R, cst::max8bit, cv::FILLED, cv::LINE_8);
	cv::Scalar summa{cv::sum(h)};
	out = h / summa[0];
}

void fftShift(const cv::Mat& in, cv::Mat& out) {
	out = in.clone();
	const int cx{out.cols / 2};
	const int cy{out.rows / 2};

	cv::Mat q0{out, cv::Rect{0, 0, cx, cy}};
	cv::Mat q1{out, cv::Rect{cx, 0, cx, cy}};
	cv::Mat q2{out, cv::Rect{0, cy, cx, cy}};
	cv::Mat q3{out, cv::Rect{cx, cy, cx, cy}};
	cv::Mat tmp{};

	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);
}

void filter2Dfreq(const cv::Mat& in, cv::Mat& out, const cv::Mat& H) {
	const std::array<cv::Mat, 2> planes{cv::Mat_<float>{in.clone()},
										cv::Mat::zeros(in.size(), CV_32F)};
	cv::Mat complexI{};
	cv::merge(planes, complexI);
	cv::dft(complexI, complexI, cv::DFT_SCALE);

	const std::array<cv::Mat, 2> planesH{cv::Mat_<float>{H.clone()},
										 cv::Mat::zeros(H.size(), CV_32F)};
	cv::Mat complexH{};
	cv::merge(planesH, complexH);
	cv::Mat complexIH;
	cv::mulSpectrums(complexI, complexH, complexIH, 0);

	cv::idft(complexIH, complexIH);
	cv::split(complexIH, planes);
	out = planes.front();
}

void computeWeinerFilter(const cv::Mat& in, cv::Mat& out, double nsr) {
	cv::Mat h_PSF_shifted{};
	fftShift(in, h_PSF_shifted);
	std::array<cv::Mat, 2> planes{cv::Mat_<float>{h_PSF_shifted.clone()},
								  cv::Mat::zeros(h_PSF_shifted.size(), CV_32F)};
	cv::Mat complexI{};
	cv::merge(planes, complexI);
	cv::dft(complexI, complexI);
	cv::split(complexI, planes);
	cv::Mat denom{};
	cv::pow(cv::abs(planes.front()), 2, denom);
	denom += nsr;
	cv::divide(planes.front(), denom, out);
}

}  // namespace beholder
