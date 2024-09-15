/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <array>
#include <cmath>

#include <opencv2/core.hpp>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>

#include "EASTDetector.h"

#include "internal/ObjDetectorBuffers.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void EASTDetector::extract()
{
	if (buf_->outs.size() != 2)
	{
		return;
	}
	//CV_CheckEQ(buf_->outs.size(), (size_t)2, "");
	cv::Mat geometry {buf_->outs[0]};
	cv::Mat confsMap {buf_->outs[1]};

	if
	(
		confsMap.dims != 4
	 || geometry.dims != 4
	 || confsMap.size[0] != 1
	 || geometry.size[0] != 1
	 || confsMap.size[1] != 1
	 || geometry.size[1] != 5
	 || confsMap.size[2] != geometry.size[2]
	 || confsMap.size[3] != geometry.size[3]
	 || confsMap.type() != CV_32FC1
	 || geometry.type() != CV_32FC1
	)
	{
		return;
	}

	const int height {confsMap.size[2]};
	const int width {confsMap.size[3]};
	for (auto y {0}; y < height; ++y)
	{
		const float* confs = confsMap.ptr<float>(0, 0, y);
		const float* x0s = geometry.ptr<float>(0, 0, y);
		const float* x1s = geometry.ptr<float>(0, 1, y);
		const float* x2s = geometry.ptr<float>(0, 2, y);
		const float* x3s = geometry.ptr<float>(0, 3, y);
		const float* angles = geometry.ptr<float>(0, 4, y);
		for (auto x {0}; x < width; ++x)
		{
			float conf {confs[x]};
			if (conf < confidenceThreshold)
			{
				continue;
			}

			float offsetX {static_cast<float>(x) * 4.0f};
			float offsetY {static_cast<float>(y) * 4.0f};
			float angle {angles[x]};
			float cosA {std::cos(angle)};
			float sinA {std::sin(angle)};
			float h {x0s[x] + x2s[x]};
			float w {x1s[x] + x3s[x]};

			cv::Point2f offset
			{
				offsetX + cosA * x1s[x] + sinA * x2s[x],
				offsetY - sinA * x1s[x] + cosA * x2s[x]
			};
			cv::Point2f p1 {cv::Point2f {-sinA * h, -cosA * h} + offset};
			cv::Point2f p3 {cv::Point2f {-cosA * w,  sinA * w} + offset};

			buf_->tBoxes.emplace_back
			(
				cv::RotatedRect
				{
					0.5f * (p1 + p3),
					cv::Size2f(w, h),
					0.0
				}.boundingRect()
			);
			buf_->tAngles.emplace_back
			(
				static_cast<double>(-angle) * 180.0 / CV_PI
			);
			buf_->tConfidences.emplace_back(conf);
		}
	}
}

void EASTDetector::store()
{
	cv::dnn::NMSBoxes
	(
		buf_->tBoxes,
		buf_->tConfidences,
		confidenceThreshold,
		nmsThreshold,
		buf_->tNMSIDs
	);

	res_.reserve(buf_->tNMSIDs.size());
	for (auto i {0ul}; i < buf_->tNMSIDs.size(); ++i)
	{
		Result r {};

		auto id {buf_->tNMSIDs[i]};	// use NMS filtered IDs to select results
		const cv::Rect& b {buf_->tBoxes[id]};

		r.boxRotAngle = buf_->tAngles[id];
		r.box = Rectangle {b.x, b.y, b.x + b.width, b.y + b.height},
		r.confidence = static_cast<double>(buf_->tConfidences[id]);

		res_.emplace_back(std::move(r));
	}
}

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
