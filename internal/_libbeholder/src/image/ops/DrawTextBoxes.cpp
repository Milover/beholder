/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <array>

#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include "DrawTextBoxes.h"
#include "OcrResults.h"
#include "ProcessingOp.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool DrawTextBoxes::execute(const cv::Mat&, cv::Mat&) const
{
	// can't really do anything without the BEHOLDER results
	return true;
}

bool DrawTextBoxes::execute
(
	const cv::Mat&,
	cv::Mat& out,
	const OcrResults& res
) const
{
	cv::Scalar c {color[0], color[1], color[2], color[3]};
	cv::Rect rect {};
	for (const auto& r : res.textBoxes)
	{
		rect.x = r.left;
		rect.y = r.top;
		rect.width = r.right - r.left;
		rect.height = r.bottom - r.top;	// because we measure from the top left
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
