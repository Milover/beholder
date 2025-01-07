/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <utility>

#include <opencv2/core.hpp>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>

#include "YOLOv8Detector.h"

#include "internal/ObjDetectorBuffers.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void YOLOv8Detector::extract()
{
	if (buf_->outs.size() != 1)
	{
		return;
	}

	cv::Mat out {buf_->outs[0]};
	if (out.dims != 3 || out.size[2] < 5)	// should have 4 coords and at least 1 class
	{
		return;
	}
	cv::transposeND(out, {0, 2, 1}, out);
	out = out.reshape(1, out.size[1]); // [1, 8400, 85] -> [8400, 85]
	cv::Mat scores {};
	for (auto i {0}; i < out.rows; ++i)
	{
		double conf;
		cv::Point maxLoc {};
		scores = out.row(i).colRange(4, out.cols);	// just a 'view', so should be efficient
		cv::minMaxLoc(scores, 0, &conf, 0, &maxLoc);

		if (conf < confidenceThreshold)
		{
			continue;
		}

		// get bbox coords; [xCenter, yCenter, width, height]
		float* det {out.ptr<float>(i)};
		buf_->tBoxes.emplace_back
		(
			cvFloor(det[0] - 0.5 * det[2]),
			cvFloor(det[1] - 0.5 * det[3]),
			cvFloor(det[2]),
			cvFloor(det[3])
		);
		buf_->tClassIDs.emplace_back(maxLoc.x);
		buf_->tConfidences.emplace_back(static_cast<float>(conf));
	}
}

void YOLOv8Detector::store()
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
		auto classID {buf_->tClassIDs[id]};

		r.box = Rectangle {b.x, b.y, b.x + b.width, b.y + b.height},
		r.confidence = static_cast<double>(buf_->tConfidences[id]);
		r.text = classes.size() > static_cast<size_t>(classID)
			   ? classes[classID] : std::to_string(classID);

		res_.emplace_back(std::move(r));
	}
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
