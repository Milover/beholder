/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <array>
#include <vector>

#include <opencv2/core/fast_math.hpp>
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
	// can't really do anything without the results
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
	cv::RotatedRect rect {};
	cv::Point2f verts[4] {};
	for (const auto& r : res)
	{
		const auto& b {r.box.cRef()};
		rect = cv::RotatedRect
		{
			cv::Point2f
			{
				static_cast<float>(b.right - b.left) / 2.0f,
				static_cast<float>(b.bottom - b.top) / 2.0f
			},
			cv::Size2f
			{
				static_cast<float>(b.right - b.left),
				static_cast<float>(b.bottom - b.top)
			},
			static_cast<float>(r.boxRotAngle)
		};
		rect.points(verts);
		for (auto i {0ul}; i < 4; ++i)
		{
			cv::line(out, verts[i], verts[(i + 1) % 4], c, thickness);
		}
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
