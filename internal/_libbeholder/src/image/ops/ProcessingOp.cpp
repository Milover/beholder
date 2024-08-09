/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <opencv2/core/mat.hpp>

#include "ProcessingOp.h"
#include "Result.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * * //

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //

bool ProcessingOp::operator()(const cv::Mat& in, cv::Mat& out) const
{
	return execute(in, out);
}


bool ProcessingOp::operator()
(
	const cv::Mat& in,
	cv::Mat& out,
	const std::vector<Result>& res
) const
{
	return execute(in, out, res);
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
