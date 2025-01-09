/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cmath>
#include <utility>

#include <opencv2/core.hpp>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>

#include "neural/PARSeqDetector.h"
#include "neural/internal/ObjDetectorBuffers.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void PARSeqDetector::extract()
{
	if (buf_->outs.size() != 1)
	{
		return;
	}
	// for the pretrained model, the output should be [1, 26, 95],
	// 25+1 positions (results) for 1+94 chars (the first char is a blank)
	const int chsetLen {static_cast<int>(charset.size())};
	cv::Mat out {buf_->outs[0]};
	if
	(
		out.size[1] != nPos_
	 || out.size[2] != chsetLen + 1
	 || out.type() != CV_32FC1
	)
	{
		return;
	}

	Result r {};
	r.confidence = 1.0;

	// TODO: pull this out, we should be able to use different decoders
	for (auto pos {0}; pos < nPos_; ++pos)
	{
		const float* pred {out.ptr<float>(0, pos)};

		int maxLoc {0};				// index of the max score
		double maxScore {pred[0]};	// (initial) max score value
		double softMaxD {0.0};		// softmax denominator (sum of exp values)
		for (auto id {0}; id < chsetLen; ++id)
		{
			double score {static_cast<double>(pred[id])};
			if (maxScore < score)
			{
				maxScore = score;
				maxLoc = id;
			}
			softMaxD += std::exp(score);
		}
		if (maxLoc > 0)
		{
			r.text += charset.at(maxLoc - 1);
			r.confidence *= std::exp(maxScore) / softMaxD;
		}
		else
		{
			// include the EOS probability as well
			r.confidence *= std::exp(maxScore) / softMaxD;
			break;
		}
	}
	if (!r.text.empty())
	{
		res_.emplace_back(std::move(r));
	}
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
