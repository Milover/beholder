/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include "Main.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace ocr {

class Config
{
	public:
		std::vector<std::string> configPaths;
		std::string modelPath;
		std::string model;

		cv::Scalar textBoxColor {0, 0, 0};
		int textBoxThickness {3};
};

struct  TBADeleter
{
	void operator()(tesseract::TessBaseAPI* p)
	{
		p->End();
		delete p;
	}
};

}

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

	//"5x5_Dots_FT_500",
	//"dotOCRDData1",		// <- this one
	//"Orario_FT_500",
	//"Transit_FT_500",
	//"Dotrice_FT_500",
	//"LCDDot_FT_500",
}

// Normalize brightness and contrast
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

// Prepare an image for OCR
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

std::unique_ptr<char*[]>
vectorStrings2UniqueCharPtr(const std::vector<std::string>& v)
{
	std::unique_ptr<char*[]> result {new char*[v.size() + 1]};

	for (auto i {0ul}; i < v.size(); ++i)
	{
		result[i] = new char[v[i].length() + 1];
		std::strcpy(result[i], v[i].c_str());
	}
	result[v.size()] = nullptr;

	return result;
}

// Setup Tesseract
std::unique_ptr<tesseract::TessBaseAPI, ocr::TBADeleter>
initialize(const ocr::Config& cfg)
{
	using tba = tesseract::TessBaseAPI;
	auto api = std::unique_ptr<tba, ocr::TBADeleter>(new tba{}, ocr::TBADeleter{});

	auto configs {vectorStrings2UniqueCharPtr(cfg.configPaths)};
	for (auto i {0ul}; i < cfg.configPaths.size(); ++i)
	{
		configs[i] = const_cast<char*>(cfg.configPaths[i].c_str());
	}
	if (
		api->Init(
			cfg.modelPath.c_str(),
			cfg.model.c_str(),
			tesseract::OEM_LSTM_ONLY,
			configs.get(),
			cfg.configPaths.size(),
			nullptr,
			nullptr,
			false
		)
	)
	{
		api.reset();
		return api;
	}
	// NOTE: would be nice if we could get it to a single line/word
	api->SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
	api->SetVariable("load_system_dawg", "0");
	api->SetVariable("load_freq_dawg", "0");
//	api->SetVariable("classify_bln_numeric_mode", "1");
//	api->SetVariable("tessedit_char_whitelist", ".,:;0123456789");

	return api;
}

// Detect text and generate text boxes
std::optional<std::vector<cv::Rect>> detectText(
	const std::unique_ptr<tesseract::TessBaseAPI, ocr::TBADeleter>& api
)
{
	std::unique_ptr<tesseract::PageIterator> iter {api->AnalyseLayout()};
	if (!iter)
	{
		return std::nullopt;
	}
	tesseract::PageIteratorLevel level {tesseract::RIL_TEXTLINE};

	// construct text boxes
	std::vector<cv::Rect> rects;
	rects.reserve(5);		// guesstimate
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

	return rects;
}

// Draw text boxes onto an image
void drawTextBoxes(
	cv::Mat& im,
	const std::vector<cv::Rect>& rects,
	const ocr::Config& cfg
)
{
	for (const auto& r : rects)
	{
		cv::rectangle(im, r, cfg.textBoxColor, cfg.textBoxThickness);
	}
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	ocr::Config cfg {};
	cfg.configPaths.emplace_back(ocr::patternConfig);
	cfg.modelPath = ocr::tessdataDir;
	cfg.model = "dotOCRDData1";

	auto api {initialize(cfg)};
	if (!api)
	{
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}

	// preprocess image
	cv::Mat im = cv::imread(ocr::testPhoto, cv::IMREAD_GRAYSCALE);
	preprocess(im);
	api->SetImage(im.data, im.cols, im.rows, 1, im.step);

	// detect text and draw text boxes
	auto rects {detectText(api)};
	if (!rects)
	{
		std::cerr << "Could not analyze text.\n";
		return 1;
	}
	drawTextBoxes(im, *rects, cfg);
	cv::imshow("result", im);
	cv::waitKey();

	// run ocr
	std::unique_ptr<char[]> text {api->GetUTF8Text()};
	std::cout << "OCR output:\n" << text.get();

	return 0;
}

// ************************************************************************* //
