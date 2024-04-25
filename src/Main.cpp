/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <iostream>
#include <memory>

#include <opencv2/opencv.hpp>	// TODO: break this out, we don't need all of it

#include "Config.h"
#include "Main.h"
#include "Ocr.h"
#include "Preprocess.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// FIXME: hardcoded stuff here
	ocr::Config cfg {};
	cfg.configPaths.emplace_back(ocr::patternConfig);
	cfg.modelPath = ocr::tessdataDir;
	cfg.model = ocr::modelName;

	auto api {ocr::initialize(cfg)};
	if (!api)
	{
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}

	// preprocess image
	cv::Mat im = cv::imread(ocr::testPhoto, cv::IMREAD_GRAYSCALE);
	ocr::preprocess(im);
	api->SetImage(im.data, im.cols, im.rows, 1, im.step);

	// detect text and draw text boxes
	auto rects {ocr::detectText(api)};
	if (!rects)
	{
		std::cerr << "Could not analyze text.\n";
		return 1;
	}
	ocr::drawTextBoxes(im, *rects, cfg);
	cv::imshow("result", im);
	cv::waitKey();

	// run ocr
	std::unique_ptr<char[]> text {api->GetUTF8Text()};
	std::cout << "OCR output:\n" << text.get();

	return 0;
}

// ************************************************************************* //
