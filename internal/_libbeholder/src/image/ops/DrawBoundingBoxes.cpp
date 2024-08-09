/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <array>
#include <vector>

#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include "DrawBoundingBoxes.h"
#include "ProcessingOp.h"
#include "Result.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool DrawBoundingBoxes::execute(const cv::Mat&, cv::Mat&) const
{
	// can't really do anything without the BEHOLDER results
	return true;
}

bool DrawBoundingBoxes::execute
(
	const cv::Mat&,
	cv::Mat& out,
	const std::vector<Result>& res
) const
{
	cv::Scalar c {color[0], color[1], color[2], color[3]};
	cv::Rect rect {};
	for (const auto& r : res)
	{
		rect.x = r.box.left;
		rect.y = r.box.top;
		rect.width = r.box.right - r.box.left;
		rect.height = r.box.bottom - r.box.top;	// because we measure from the top left
		cv::rectangle(out, rect, c, thickness);
	}
	return true;
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
