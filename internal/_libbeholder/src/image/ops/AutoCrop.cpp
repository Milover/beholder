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

#include "AutoCrop.h"
#include "Result.h"
#include "ProcessingOp.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool AutoCrop::execute(const cv::Mat& in, cv::Mat& out) const
{
	cv::RotatedRect box {};
	cv::Point2f center {};
	bool ok {executeImpl(in, out, box, center)};
	if (!ok)
	{
		return false;
	}

	// crop (now centered on the image)
	cv::Rect crop
	{
		cv::RotatedRect {center, box.size, 0}.boundingRect()
	};
	// snap to bounds
	crop.x = crop.x > 0 ? crop.x : 0;
	crop.x = crop.x < out.cols ? crop.x : out.cols - 1;
	crop.width = crop.x + crop.width <= out.cols ? crop.width : out.cols - crop.x;

	crop.y = crop.y > 0 ? crop.y : 0;
	crop.y = crop.y < out.rows ? crop.y : out.rows - 1;
	crop.height = crop.y + crop.height <= out.rows ? crop.height : out.rows - crop.y;

	out = out(crop);
	return true;
}

bool AutoCrop::execute
(
	const cv::Mat& in,
	cv::Mat& out,
	const std::vector<Result>&
) const
{
	return execute(in, out);
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
