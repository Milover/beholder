/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <iostream>
#include <memory>
#include <string>

#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include "Main.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace ocr {

	// FIXME: hardcoded traineddata dir
	static const char* const trainedDataDir
	{
		"/Users/philipp/Documents/c++/libs/tesseract/tessdata/fast"
	};

	// FIXME: hardcoded test image
	static const char* const testPhoto
	{
		"/Users/philipp/Documents/c++/projects/ocr/test/testdata/simple_12pt.tif"
	};
}

using tAPI = tesseract::TessBaseAPI;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// tesseract setup
	auto tAPIDel = [](tAPI* p) {
		p->End();
		delete p;
	};
	auto api = std::unique_ptr<tAPI, decltype(tAPIDel)>(new tAPI{}, tAPIDel);
	if (api->Init(ocr::trainedDataDir, "eng", tesseract::OEM_LSTM_ONLY)) {
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}
	// FIXME: should get it to single line/word or something similar
	api->SetPageSegMode(tesseract::PSM_AUTO);

	// preprocess image
	cv::Mat im = cv::imread(ocr::testPhoto, cv::IMREAD_GRAYSCALE);
	api->SetImage(im.data, im.cols, im.rows, 1, im.step);

	// run ocr
	std::string outText = std::string(api->GetUTF8Text());
	std::cout << "OCR output:\n" << outText;

	return 0;
}

// ************************************************************************* //
