// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/image/ops/AutoOrient.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"
#include "beholder/util/Constants.h"

namespace beholder {

bool AutoOrient::execute(const cv::Mat& in, cv::Mat& out) const {
	cv::RotatedRect box{};
	cv::Point2f center{};

	return executeImpl(in, out, box, center);
}

bool AutoOrient::execute(
	const cv::Mat& in, cv::Mat& out,
	[[maybe_unused]] const std::vector<Result>& res) const {
	return execute(in, out);
}

bool AutoOrient::executeImpl(const cv::Mat& in, cv::Mat& out,
							 cv::RotatedRect& box, cv::Point2f& center) const {
	findTextBox(in, kernelSize, textHeight, textWidth, padding,
				gradientKernelSize,
				box	 // the return value
	);

	const cv::Point2f ctr{box.center};
	// determine bounding rectangle, center not relevant
	cv::Rect2f bbox{};
	{
		const cv::Rect2f imgBox{
			cv::RotatedRect{ctr, in.size(), box.angle}.boundingRect2f()};
		const cv::Rect2f boxBox	 // because the box is padded
			{cv::RotatedRect{ctr, box.size, 0}.boundingRect2f()};
		bbox.width = imgBox.width > boxBox.width ? imgBox.width : boxBox.width;
		bbox.height =
			imgBox.height > boxBox.height ? imgBox.height : boxBox.height;
	}
	// adjust transformation matrix by adding a translation from the
	// center of rotation to the (new) image center,
	// i.e. center the text box on the image
	cv::Mat rot{cv::getRotationMatrix2D(ctr, box.angle, 1.0)};
	center = cv::Point2f{(bbox.width - 1) / 2, (bbox.height - 1) / 2};
	const cv::Point2f shift{center - ctr};
	rot.at<double>(0, 2) += static_cast<double>(shift.x);
	rot.at<double>(1, 2) += static_cast<double>(shift.y);

	cv::warpAffine(in, out, rot, bbox.size(), cv::INTER_LINEAR,
				   cv::BORDER_CONSTANT, cv::Scalar::all(padValue));

	return true;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void findTextBox(const cv::Mat& in, int kSize, float txtHeight, float txtWidth,
				 float padding, int gKSize, cv::RotatedRect& returnValue) {
	constexpr auto deg90{90.0F};

	cv::Mat img{};
	if (in.channels() > 1) {
		cv::cvtColor(in, img, cv::COLOR_BGR2GRAY, 1);
	} else {
		in.copyTo(img);
	}
	cv::RotatedRect box{};
	cv::Mat kernel;

	// TODO: either one seems ok, should test speed/robustness, but can
	// keep both either way
	//cv::Sobel(img, img, CV_8U, 1, 0, gKSize, 1, 0, cv::BORDER_DEFAULT);
	kernel =
		cv::getStructuringElement(cv::MORPH_RECT, cv::Size(gKSize, gKSize));
	cv::morphologyEx(img, img, cv::MORPH_GRADIENT, kernel);

	cv::threshold(img, img, 0, cst::max8bit,
				  cv::THRESH_BINARY + cv::THRESH_OTSU);
	kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kSize, kSize));
	cv::morphologyEx(img, img, cv::MORPH_CLOSE, kernel);

	std::vector<std::vector<cv::Point>> contours;
	contours.reserve(20);  // NOLINT(*-magic-numbers): guesstimate
	cv::findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	for (const auto& c : contours) {
		cv::RotatedRect rect{cv::minAreaRect(c)};
		// we assume that width > height, and do this here
		// to reduce confusion when defining runtime parameters
		if (rect.size.width < rect.size.height) {
			rect = cv::RotatedRect{
				rect.center, cv::Size2f{rect.size.height, rect.size.width},
				rect.angle - deg90	// FIXME: this is probably bad
			};
		}
		if (rect.size.width > txtWidth && rect.size.height > txtHeight) {
			if (rect.size.area() > box.size.area()) {
				box = rect;
			}
		}
	}

	returnValue = cv::RotatedRect{
		box.center,
		cv::Size2f{box.size.width + 2 * padding, box.size.height + 2 * padding},
		box.angle};
}

}  // namespace beholder
