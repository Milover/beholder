/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <vector>

#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include "capi/Result.h"
#include "image/ProcessingOp.h"
#include "image/ops/ResizeToHeight.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool ResizeToHeight::execute(const cv::Mat& in, cv::Mat& out) const
{
	double ratio
	{
		static_cast<double>(in.cols)
	  / static_cast<double>(in.rows)
	};
	int width {cvRound(height * ratio)};

	// we usually shrink images
	int interp {cv::INTER_AREA};
	if (width*height > in.rows*in.cols)
	{
		// when we're enlarging, usually the quality is already pretty bad
		// and the image should be pretty small so INTER_CUBIC should
		// in general be better for our use cases than INTER_LINEAR even
		// though it's slower
		interp = cv::INTER_CUBIC;
	}

	cv::resize(in, out, cv::Size {width, height}, 0, 0, interp);
	return true;
}

bool ResizeToHeight::execute
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
