/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

#include "AddPadding.h"
#include "Result.h"
#include "ProcessingOp.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool AddPadding::execute(const cv::Mat& in, cv::Mat& out) const
{
	cv::Mat tmp {in.rows + 2*padding, in.cols + 2*padding, in.depth()};
	// assume white background
	cv::copyMakeBorder
	(
		in,
		tmp,
		padding,
		padding,
		padding,
		padding,
		cv::BORDER_ISOLATED,
		cv::Scalar {255, 255, 255}
	);
	out = tmp;
	return true;
}

bool AddPadding::execute
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
