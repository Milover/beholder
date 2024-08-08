/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>

#include "Crop.h"
#include "Result.h"
#include "ProcessingOp.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool Crop::execute(const cv::Mat& in, cv::Mat& out) const
{
	cv::Rect crop {left, top, width, height};
	// snap to bounds
	crop.x = crop.x > 0 ? crop.x : 0;
	crop.x = crop.x < in.cols ? crop.x : in.cols - 1;
	crop.width = crop.x + crop.width <= in.cols ? crop.width : in.cols - crop.x;

	crop.y = crop.y > 0 ? crop.y : 0;
	crop.y = crop.y < in.rows ? crop.y : in.rows - 1;
	crop.height = crop.y + crop.height <= in.rows ? crop.height : in.rows - crop.y;

	out = out(crop);
	return true;
}

bool Crop::execute
(
	const cv::Mat& in,
	cv::Mat& out,
	const Result&
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
