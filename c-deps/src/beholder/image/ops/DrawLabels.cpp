/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <array>
#include <iomanip>
#include <sstream>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include "capi/Result.h"
#include "image/ProcessingOp.h"
#include "image/ops/DrawLabels.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool DrawLabels::execute(const cv::Mat&, cv::Mat&) const
{
	// can't really do anything without the BEHOLDER results
	return true;
}

bool DrawLabels::execute
(
	const cv::Mat&,
	cv::Mat& out,
	const std::vector<Result>& res
) const
{
	cv::Scalar c {color[0], color[1], color[2], color[3]};
	std::string label;
	for (const auto& r : res)
	{
		std::stringstream ss;
		ss << r.text << ": "
		   << std::fixed << std::setprecision(2) << r.confidence;
		label = ss.str();

		int baseline;
		cv::Size labelSize
		{
			cv::getTextSize
			(
				label, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline
			)
		};
		int top {cv::max(r.box.cRef().top, labelSize.height)};
		cv::putText
		(
			out, label,
			cv::Point(r.box.cRef().left, top - baseline),
			cv::FONT_HERSHEY_SIMPLEX, fontScale, c, thickness
		);
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
