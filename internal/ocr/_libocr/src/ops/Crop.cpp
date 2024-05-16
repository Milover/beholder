/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>

#include "Crop.h"
#include "OcrResults.h"
#include "ProcessingOp.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool Crop::execute(const cv::Mat& in, cv::Mat& out) const
{
	// snap to bounds
	auto l {left > 0 ? left : 0};
	auto r {right <= in.cols ? right : in.cols};
	auto t {top > 0 ? top : 0};
	auto b {bottom <= in.rows ? bottom : in.rows};
	// sanity check
	if (l >= r)
	{
		if (l == 0) { r = in.cols; } else { l = r - 1; }
	}
	if (t >= b)
	{
		if (t == 0) { b = in.rows; } else { t = b - 1; }
	}
	cv::Range cols(l, r);
	cv::Range rows(t, b);
	out = in(rows, cols);
	return true;
}

bool Crop::execute
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
