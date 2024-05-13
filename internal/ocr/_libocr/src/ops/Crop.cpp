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
	if (l > r)
	{
		l = r;
	}
	if (t > b)
	{
		t = b;
	}
	cv::Range cols(l, r);
	cv::Range rows(t, b);
	out = in(rows, cols);
	// can never fail, because even if l = r, or t = b, there will always
	// be at least 1 row and 1 column
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
