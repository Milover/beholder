/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <vector>

#include <opencv2/opencv.hpp>	// TODO: break this out, we don't need all of it

#include "Preprocess.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

void normalize(cv::Mat& im, float clipPct)
{
	// compute histogram
	std::vector<cv::Mat> input {im};
	std::vector<int> channels {0};
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
	clipPct *= max / 100.0;	// ?
	clipPct /= 2.0;			// ?

	// FIXME: this is looks kinda dumb
	// locate left cut
	int min_gray {0};
	while (acc[min_gray] < clipPct)
	{
		++min_gray;
	}

	int max_gray {histSize[0] - 1};
	while (acc[max_gray] >= (max - clipPct))
	{
		--max_gray;
	}

	float alpha {255.0f / static_cast<float>((max_gray - min_gray))};
	float beta {-min_gray * alpha};

	cv::convertScaleAbs(im, im, alpha, beta);
}

void preprocess(cv::Mat& im)
{
	// XXX:  should also crop here
	cv::resize(im, im, cv::Size(860, 430));

//	cv::Mat im = img.clone();
//	cv::bitwise_not(im, im);

	normalize(im);

//	cv::Sobel(im, im, CV_8U, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);

	cv::medianBlur(im, im, 3);
	//cv::GaussianBlur(im, im, cv::Size(3, 3), 0);

	cv::threshold(im, im, 0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU);
	//cv::threshold(im, im, 100, 255, cv::THRESH_BINARY);

	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::morphologyEx(im, im, cv::MORPH_OPEN, element, cv::Point{-1, -1}, 5);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
