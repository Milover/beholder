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
#include "Postprocess.h"
#include "Preprocess.h"
#include "TimeAnalyzer.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

enum class Times
{
	start,
	setup,
	load,
	prep,
	ocr,
	postp
};

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	using TS = ocr::Times;

	ocr::TimeAnalyzer<TS> ta {};
	ta.mark(TS::start);

	// setup the config
	// FIXME: hardcoded stuff here
	ocr::Config cfg {};
	cfg.configPaths.emplace_back(ocr::patternConfig);
	cfg.modelPath = ocr::tessdataDir;
	cfg.model = ocr::modelName;

	// setup tesseract
	auto api {ocr::initialize(cfg)};
	if (!api)
	{
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}
	ta.mark(TS::setup);

	// read/load an image
	cv::Mat im = cv::imread(ocr::testPhoto, cv::IMREAD_GRAYSCALE);
	ta.mark(TS::load);

	// preprocess the image
	ocr::preprocess(im);
	api->SetImage(im.data, im.cols, im.rows, 1, im.step);
	ta.mark(TS::prep);

	// NOTE: it seems that GetUTF8Text uses this analysis and does not
	// run it again (are we sure?)
	//
	// detect text and draw text boxes
	auto rects {ocr::detectText(api)};
	if (!rects)
	{
		// If text detection fails, text recognition will also fail, so it's
		// fine to exit right away.
		std::cerr << "Could not analyze text.\n";
		return 1;
	}
	// run ocr
	std::unique_ptr<char[]> text {api->GetUTF8Text()};
	ta.mark(TS::ocr);

	// postprocess
	ocr::drawBoxes(im, *rects, cfg);
	ta.mark(TS::postp);

	// show results
	std::cout << "OCR output:\n" << text.get();
	std::cout << "setup time:       " << ta.duration(TS::setup) << "ms\n"
			  << "image load time:  " << ta.duration(TS::setup, TS::load) << "ms\n"
			  << "preprocess time:  " << ta.duration(TS::load, TS::prep) << "ms\n"
			  << "ocr time:         " << ta.duration(TS::prep, TS::ocr) << "ms\n"
			  << "postprocess time: " << ta.duration(TS::ocr, TS::postp) << "ms\n"
			  << "process time:     " << ta.duration(TS::setup, TS::postp) << "ms\n"
			  << "total time:       " << ta.duration(TS::postp) << "ms\n"
			  << '\n';
	cv::imshow("result", im);
	cv::waitKey();

	return 0;
}

// ************************************************************************* //
