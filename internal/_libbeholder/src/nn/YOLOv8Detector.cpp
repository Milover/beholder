/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

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
	cv::transposeND(buf_->outs[0], {0, 2, 1}, buf_->outs[0]);

	cv::Mat scores {};
	cv::Mat out {buf_->outs[0]};
	out = out.reshape(1, out.size[1]); // [1, 8400, 85] -> [8400, 85]
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

	cv::dnn::NMSBoxes
	(
		buf_->tBoxes,
		buf_->tConfidences,
		confidenceThreshold,
		nmsThreshold,
		buf_->tNMSIDs
	);
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
