/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cmath>
#include <vector>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "neural/CRAFTDetector.h"
#include "neural/internal/ObjDetectorBuffers.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void CRAFTDetector::extract()
{
	if (buf_->outs.size() != 2)
	{
		return;
	}
	cv::Mat out {};
	transposeND(buf_->outs[0], std::vector<int>{0, 3, 1, 2}, out);
	cv::Mat textmap {out.size[2], out.size[3], CV_32F, out.ptr<float>(0, 0)};
	cv::Mat linkmap {out.size[2], out.size[3], CV_32F, out.ptr<float>(0, 1)};

	/* TODO: do some assertions here
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
	*/

	cv::Mat textScore {};
	cv::Mat linkScore {};
	cv::threshold(textmap, textScore, lowText, 1.0, cv::THRESH_BINARY);
	cv::threshold(linkmap, linkScore, linkThreshold, 1.0, cv::THRESH_BINARY);
	cv::Mat linkAreaMask {(linkScore == 1) & (textScore == 0)};
	// clamp
	cv::Mat textScoreComb {textScore + linkScore};
	for (auto i {0ul}; i < textScoreComb.total(); ++i)
	{
		if (textScoreComb.at<float>(static_cast<int>(i)) < 0.0f)
		{
			textScoreComb.at<float>(static_cast<int>(i)) = 0.0f;
		}
		else if (textScoreComb.at<float>(static_cast<int>(i)) > 1.0f)
		{
			textScoreComb.at<float>(static_cast<int>(i)) = 1.0f;
		}
	}
	cv::Mat textScoreCombU {};
	textScoreComb.convertTo(textScoreCombU, CV_8U);

	cv::Mat labels {};
	cv::Mat stats {};
	cv::Mat centroids {};
	int nLabels
	{
		cv::connectedComponentsWithStats
		(
			textScoreCombU, labels, stats, centroids, 4, CV_32S
		)
	};

	// skip the first label, because it will be the whole image
	for (auto i {1}; i < nLabels; ++i)
	{
		cv::Mat labelmask {labels == i};

		// size filtering
		int statSize {stats.at<int>(i, cv::CC_STAT_AREA)};
		if (statSize < 10)
		{
			continue;
		}
		// thresholding
		double maxVal;
		cv::minMaxLoc(textmap, nullptr, &maxVal, nullptr, nullptr, labelmask);
		if (maxVal < textThreshold)
		{
			continue;
		}
		// create segmentation map
		cv::Mat segmap {cv::Mat::zeros(textmap.size(), CV_8U)};
		segmap.setTo(255, labelmask);
		segmap.setTo(0, linkAreaMask);
		int x {stats.at<int>(i, cv::CC_STAT_LEFT)};
		int y {stats.at<int>(i, cv::CC_STAT_TOP)};
		int w {stats.at<int>(i, cv::CC_STAT_WIDTH)};
		int h {stats.at<int>(i, cv::CC_STAT_HEIGHT)};
		int nIter
		{
			cvFloor
			(
				2.0 * std::sqrt
				(
					static_cast<double>(statSize * std::min(w, h))
				  / static_cast<double>(w * h)
				)
			)
		};
		int sx {x - nIter};
		int ex {x + w + nIter + 1};
		int sy {y - nIter};
		int ey {y + h + nIter + 1};
		// boundary check
		sx = sx < 0 ? 0 : sx;
		sy = sy < 0 ? 0 : sy;
		ex = ex >= textmap.cols ? textmap.cols : ex;
		ey = ey >= textmap.rows ? textmap.rows : ey;
		cv::Mat kernel
		{
			cv::getStructuringElement(cv::MORPH_RECT, cv::Size {1 + nIter, 1 + nIter})
		};
		cv::Mat segmapROI {segmap(cv::Range {sy, ey}, cv::Range{sx, ex})};
		cv::dilate(segmapROI, segmapROI, kernel);
		// make box
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(segmap, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		for (const auto& c : contours)
		{
			cv::RotatedRect rect {cv::minAreaRect(c)};
			// The model seems to output results at 1/2 scale.
			// For more info, see:
			//	- https://arxiv.org/pdf/1904.01941
			//	- https://github.com/clovaai/CRAFT-pytorch/blob/e332dd8b718e291f51b66ff8f9ef2c98ee4474c8/craft_utils.py#L237
			//
			// FIXME: the boxes seem to get arbitrarily oriented, or
			// we're just too stupid to understand the logic
			rect.size = cv::Size2f
			{
				rect.size.width * 2.0f,
				rect.size.height * 2.0f
			};
			rect.center *= 2.0f;
			if (rect.size.width < rect.size.height)
			{
				rect.size = cv::Size2f {rect.size.height, rect.size.width};
				rect.angle -= 90.0f;
			}
			buf_->tBoxes.emplace_back
			(
				cv::RotatedRect
				{
					rect.center,
					rect.size,
					0.0
				}.boundingRect()
			);
			buf_->tAngles.emplace_back(rect.angle);
			buf_->tConfidences.emplace_back(0.0);
		}
	}
}

void CRAFTDetector::store()
{
	res_.reserve(buf_->tBoxes.size());
	for (auto i {0ul}; i < buf_->tBoxes.size(); ++i)
	{
		Result r {};

		const cv::Rect& b {buf_->tBoxes[i]};

		r.boxRotAngle = buf_->tAngles[i];
		r.box = Rectangle {b.x, b.y, b.x + b.width, b.y + b.height},
		r.confidence = static_cast<double>(buf_->tConfidences[i]);

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
