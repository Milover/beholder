/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "Config.h"
#include "Utility.h"

#include "ImageProcessor.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void ImageProcessor::drawRectangles
(
	const std::vector<cv::Rect>& rects,
	const Config& cfg
)
{
	for (const auto& r : rects)
	{
		cv::rectangle(
			img_,
			r,
			arr2CvScalar(&cfg.textBoxColor),
			cfg.textBoxThickness
		);
	}
}


const cv::Mat& ImageProcessor::getImage() const
{
	return img_;
}


void ImageProcessor::normalize(float clipPct)
{
	// compute histogram
	std::vector<cv::Mat> input {img_};
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

	// locate right cut
	int max_gray {histSize[0] - 1};
	while (acc[max_gray] >= (max - clipPct))
	{
		--max_gray;
	}

	float alpha {255.0f / static_cast<float>((max_gray - min_gray))};
	float beta {-min_gray * alpha};

	cv::convertScaleAbs(img_, img_, alpha, beta);
}


void ImageProcessor::preprocess()
{
	// XXX:  should also crop here
	cv::resize(img_, img_, cv::Size(860, 430));

//	cv::Mat im = img.clone();
//	cv::bitwise_not(im, im);

	normalize();

//	cv::Sobel(img_, img_, CV_8U, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);

	cv::medianBlur(img_, img_, 3);
	//cv::GaussianBlur(img_, img_, cv::Size(3, 3), 0);

	cv::threshold(img_, img_, 0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU);
	//cv::threshold(img_, img_, 100, 255, cv::THRESH_BINARY);

	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::morphologyEx(img_, img_, cv::MORPH_OPEN, element, cv::Point{-1, -1}, 5);
}

void ImageProcessor::readImage(const std::string& path, int flags)
{
	img_ = cv::imread(path, flags);
}


void ImageProcessor::showImage(const std::string& title) const
{
	cv::imshow(title, img_);
	cv::waitKey();
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
