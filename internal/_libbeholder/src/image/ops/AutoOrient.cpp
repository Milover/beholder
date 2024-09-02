/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <vector>

#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include "AutoOrient.h"
#include "Result.h"
#include "ProcessingOp.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool AutoOrient::execute(const cv::Mat& in, cv::Mat& out) const
{
	cv::RotatedRect box {};
	cv::Point2f center {};
	bool ok {executeImpl(in, out, box, center)};

	return ok;
}

bool AutoOrient::execute
(
	const cv::Mat& in,
	cv::Mat& out,
	const std::vector<Result>&
) const
{
	return execute(in, out);
}

bool AutoOrient::executeImpl
(
	const cv::Mat& in,
	cv::Mat& out,
	cv::RotatedRect& box,
	cv::Point2f& center
) const
{
	findTextBox
	(
		in,
		kernelSize,
		textHeight,
		textWidth,
		padding,
		gradientKernelSize,
		box	// the return value
	);

	const cv::Point2f ctr {box.center};
	// determine bounding rectangle, center not relevant
	cv::Rect2f bbox {};
	{
		cv::Rect2f imgBox
		{
			cv::RotatedRect {ctr, in.size(), box.angle}.boundingRect2f()
		};
		cv::Rect2f boxBox	// because the box is padded
		{
			cv::RotatedRect {ctr, box.size, 0}.boundingRect2f()
		};
		bbox.width = imgBox.width > boxBox.width ? imgBox.width : boxBox.width;
		bbox.height = imgBox.height > boxBox.height ? imgBox.height : boxBox.height;
	}
	// adjust transformation matrix by adding a translation from the
	// center of rotation to the (new) image center,
	// i.e. center the text box on the image
	cv::Mat rot {cv::getRotationMatrix2D(ctr, box.angle, 1.0)};
	center = cv::Point2f
	{
		0.5f*(bbox.width - 1),
		0.5f*(bbox.height - 1)
	};
	cv::Point2f shift {center - ctr};
	rot.at<double>(0, 2) += static_cast<double>(shift.x);
	rot.at<double>(1, 2) += static_cast<double>(shift.y);

	// assume a white background
	cv::warpAffine
	(
		in,
		out,
		rot,
		bbox.size(),
		cv::INTER_NEAREST,
		cv::BORDER_CONSTANT,
		cv::Scalar{255, 255, 255}
	);

	return true;
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

void findTextBox
(
	const cv::Mat& in,
	int kSize,
	float txtHeight,
	float txtWidth,
	float padding,
	int gKSize,
	cv::RotatedRect& returnValue
)
{
	cv::Mat img {in.clone()};
	cv::RotatedRect box {};
	cv::Mat kernel;

	// TODO: either one seems ok, should test speed/robustness, but can
	// keep both either way
	//cv::Sobel(img, img, CV_8U, 1, 0, gKSize, 1, 0, cv::BORDER_DEFAULT);
	kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(gKSize, gKSize));
	cv::morphologyEx(img, img, cv::MORPH_GRADIENT, kernel);

	cv::threshold(img, img, 0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU);
	kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kSize, kSize));
	cv::morphologyEx(img, img, cv::MORPH_CLOSE, kernel);

	std::vector<std::vector<cv::Point>> contours;
	contours.reserve(20);	// guesstimate
	cv::findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	for(const auto& c : contours)
	{
		cv::RotatedRect rect {cv::minAreaRect(c)};
		// we assume that width > height, and to this here
		// to reduce confusion when defining runtime parameters
		if (rect.size.width < rect.size.height)
		{
			rect = cv::RotatedRect
			{
				rect.center,
				cv::Size2f {rect.size.height, rect.size.width},
				rect.angle - 90.0f		// FIXME: this is probably bad
			};
		}
		if (rect.size.width > txtWidth && rect.size.height > txtHeight)
		{
			if (rect.size.area() > box.size.area())
			{
				box = rect;
			}
		}
	}

	returnValue = cv::RotatedRect
	{
		box.center,
		cv::Size2f {box.size.width + 2*padding, box.size.height + 2*padding},
		box.angle
	};
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
