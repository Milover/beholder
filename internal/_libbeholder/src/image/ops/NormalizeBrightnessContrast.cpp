/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include "NormalizeBrightnessContrast.h"
#include "ProcessingOp.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool NormalizeBrightnessContrast::execute(const cv::Mat& in, cv::Mat& out) const
{
	return normalizeBrightnessContrast(in, out, clipPct);
}

bool NormalizeBrightnessContrast::execute
(
	const cv::Mat& in,
	cv::Mat& out,
	const std::vector<Result>&
) const
{
	return execute(in, out);
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

bool normalizeBrightnessContrast
(
	const cv::Mat& in,
	cv::Mat& out,
	float clipPct
)
{
	// compute histogram
	std::vector<cv::Mat> input {in};	// FIXME: this is wasteful
	std::vector<int> channels {0};		// FIXME: read from image
	std::vector<int> histSize {255};
	cv::Mat hist;

	cv::calcHist(input, channels, cv::Mat{}, hist, histSize, std::vector<float>{});

	// compute cumulative distribution
	std::vector<float> acc;
	acc.reserve(hist.total());
	acc.emplace_back(hist.at<float>(0));
	for (auto i {1ul}; i < hist.total(); ++i)
	{
		acc.emplace_back(acc[i-1] + hist.at<float>(i));
	}

	// locate clip points
	float max {acc.back()};
	clipPct *= max / 100.0;	// convert from pct. to actual value
	clipPct /= 2.0;			// because we clip from both sides

	// FIXME: this is looks kinda dumb
	// locate left cut
	int min_gray {0};
	while (acc[min_gray] < clipPct)
	{
		++min_gray;
	}

	// locate right cut
	int max_gray {histSize[0] - 1};
	while (acc[max_gray] >= (max - clipPct))
	{
		--max_gray;
	}

	float alpha {255.0f / static_cast<float>((max_gray - min_gray))};
	float beta {-min_gray * alpha};

	cv::convertScaleAbs(in, out, alpha, beta);

	return true;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
