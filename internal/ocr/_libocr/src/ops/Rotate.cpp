/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include "Rotate.h"
#include "OcrResults.h"
#include "ProcessingOp.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool Rotate::execute(const cv::Mat& in, cv::Mat& out) const
{
	// get rotation matrix for rotating the image
	// around it's center in pixel coordinates
	cv::Point2f center
	{
		0.5f*static_cast<float>(in.cols - 1),
		0.5f*static_cast<float>(in.rows - 1)
	};
	cv::Mat rot {cv::getRotationMatrix2D(center, angle, 1.0)};

	// determine bounding rectangle, center not relevant
	cv::Rect2f bbox
	{
		cv::RotatedRect {center, in.size(), angle}.boundingRect2f()
	};

	// adjust transformation matrix
	rot.at<double>(0, 2) += 0.5*static_cast<double>(bbox.width - in.cols);
	rot.at<double>(1, 2) += 0.5*static_cast<double>(bbox.height - in.rows);

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

bool Rotate::execute
(
	const cv::Mat& in,
	cv::Mat& out,
	const OcrResults&
) const
{
	return execute(in, out);
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
