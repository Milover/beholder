/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include "Main.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace ocr {

	// FIXME: hardcoded traineddata dir
	static constexpr char const* tessdataDir
	{
		"/Users/philipp/Documents/c++/projects/ocr/assets/tessdata/dotmatrix"
	};

	// FIXME: hardcoded traineddata dir
	static constexpr char const* patternConfig
	{
		"/Users/philipp/Documents/c++/projects/ocr/assets/test.patterns.config"
	};

	// FIXME: hardcoded test image
	static constexpr char const* testPhoto
	{
		//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/simple_12pt.tif"
		//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.0.raw.jpg"
		//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.0.clean.jpg"
		//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.1.png"
		//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.2.jpg"
		//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.3.jpeg"
		//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.3.cropped.jpg"
		//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.4.png"
		//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.5.png"
		//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.6.jpeg"
		"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.7.bmp"
		//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/random.jpg"
	};
}

void normalize(cv::Mat& im, float clipPct = 0.5)
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

cv::Mat preprocess(const cv::Mat& img)
{
	cv::Mat im = img.clone();
//	cv::bitwise_not(im, im);
//	cv::imshow("Orig", im);
//	cv::waitKey();

	normalize(im);
	cv::imshow("Norm", im);
	cv::waitKey();

//	cv::Sobel(im, im, CV_8U, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
//	cv::imshow("Sobel", im);
//	cv::waitKey();

	cv::medianBlur(im, im, 3);
	//cv::GaussianBlur(im, im, cv::Size(3, 3), 0);
	cv::imshow("Median", im);
	cv::waitKey();

	cv::threshold(im, im, 0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU);
	//cv::threshold(im, im, 100, 255, cv::THRESH_BINARY);
	cv::imshow("Threshold", im);
	cv::waitKey();

	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::morphologyEx(im, im, cv::MORPH_OPEN, element, cv::Point{-1, -1}, 5);
	cv::imshow("Close", im);
	cv::waitKey();

	return im;
}

using tAPI = tesseract::TessBaseAPI;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// tesseract setup
	char* configs[] = {const_cast<char*>(ocr::patternConfig)};
	int configSize {1};
	auto tAPIDel = [](tAPI* p)
	{
		p->End();
		delete p;
	};
	auto api = std::unique_ptr<tAPI, decltype(tAPIDel)>(new tAPI{}, tAPIDel);
	if (
		api->Init(
			ocr::tessdataDir,
			//"5x5_Dots_FT_500",
			"dotOCRDData1",
			//"Orario_FT_500",
			//"Transit_FT_500",
			//"Dotrice_FT_500",
			//"LCDDot_FT_500",
			tesseract::OEM_LSTM_ONLY,
			configs,
			configSize,
			nullptr,
			nullptr,
			false
		)
	)
	{
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}
	// FIXME: should get it to single line/word or something similar
	api->SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
	api->SetVariable("load_system_dawg", "0");
	api->SetVariable("load_freq_dawg", "0");
//	api->SetVariable("classify_bln_numeric_mode", "1");
//	api->SetVariable("tessedit_char_whitelist", ".,:;0123456789");

	// preprocess image
	cv::Mat img = cv::imread(ocr::testPhoto, cv::IMREAD_GRAYSCALE);
	cv::resize(img, img, cv::Size(860, 430));
	cv::Mat im = preprocess(img);
	api->SetImage(im.data, im.cols, im.rows, 1, im.step);

	// detect text
	tesseract::PageIterator* iter {api->AnalyseLayout()};
	tesseract::PageIteratorLevel level {tesseract::RIL_TEXTLINE};
	if (!iter)
	{
		std::cerr << "Could not analyse layout.\n";
		return 1;
	}

	// draw contours
	std::vector<cv::Rect> rects;
	do
	{
		int left, top, right, bottom;
		if (iter->BoundingBox(level, &left, &top, &right, &bottom))
		{
			rects.emplace_back(
				cv::Point{left, top}, cv::Point{right, bottom}
			);
		}
	} while(iter->Next(level));

	// FIXME: unnecessary, move into loop above
	for (const auto& r : rects)
	{
		cv::rectangle(im, r, cv::Scalar(0, 0, 0), 3);
	}
	cv::imshow("tesseract result", im);
	cv::waitKey();

	// run ocr
	std::string outText = std::string(api->GetUTF8Text());
	std::cout << "OCR output:\n" << outText;

	return 0;
}

// ************************************************************************* //
