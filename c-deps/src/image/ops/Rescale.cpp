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

#include "Rescale.h"
#include "Result.h"
#include "ProcessingOp.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool Rescale::execute(const cv::Mat& in, cv::Mat& out) const
{
	// we usually shrink images
	int interp {cv::INTER_AREA};
	if (scale > 1.0)
	{
		// when we're enlarging, usually the quality is already pretty bad
		// and the image should be pretty small so INTER_CUBIC should
		// in general be better for our use cases than INTER_LINEAR even
		// though it's slower
		interp = cv::INTER_CUBIC;
	}
	cv::resize(in, out, cv::Size {}, scale, scale, interp);
	return true;
}

bool Rescale::execute
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
