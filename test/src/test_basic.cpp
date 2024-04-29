/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#include <opencv2/opencv.hpp>

#include "Config.h"
#include "Ocr.h"
#include "Postprocess.h"
#include "Preprocess.h"
#include "TimeAnalyzer.h"
#include "Utility.h"

#include "Test.h"

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
	postp,
	end
};

[[maybe_unused]] inline static constexpr std::size_t nIter {1000};

double stableSum(const std::vector<double>& vals)
{
	double sum {0.0};
	double c {0.0};
	double y {0.0};
	double t {0.0};


	for (const auto& v : vals)
	{
		y = v - c;
		t = sum + y;
		c = (t - sum) - y;
		sum = t;
	}
	return sum;
}

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
	std::unique_ptr<char[]> textC {api->GetUTF8Text()};
	std::string text {textC.get()};
	ocr::trimWhiteLR(text);
	ta.mark(TS::ocr);

	// postprocess
	ocr::drawBoxes(im, *rects, cfg);
	ta.mark(TS::postp);

	// show results
	std::string expected {"V20000229"};
	std::cout << "Expected output:  " << expected << '\n';

	std::cout << "OCR output:       " << text << '\n';
	std::cout << "setup time:       " << ta.duration(TS::setup) << "ms\n"
			  << "image load time:  " << ta.duration(TS::setup, TS::load) << "ms\n"
			  << "preprocess time:  " << ta.duration(TS::load, TS::prep) << "ms\n"
			  << "ocr time:         " << ta.duration(TS::prep, TS::ocr) << "ms\n"
			  << "postprocess time: " << ta.duration(TS::ocr, TS::postp) << "ms\n"
			  << "process time:     " << ta.duration(TS::setup, TS::postp) << "ms\n"
			  << "total time:       " << ta.duration(TS::postp) << "ms\n"
			  << '\n';
	//cv::imshow("result", im);
	//cv::waitKey();

	return expected == text;
}

/*
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	using TS = ocr::Times;

	std::vector<double> times;
	times.reserve(ocr::nIter);
	ocr::TimeAnalyzer<TS> ta {};

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

	// read/load an image
	cv::Mat img = cv::imread(ocr::testPhoto, cv::IMREAD_GRAYSCALE);
	cv::Mat im = img.clone();

	for (auto i {0ul}; i < ocr::nIter; ++i)
	{
		std::cout << "current i: " << i << std::flush << '\r';
		// reset
		api->Clear();
		im = img.clone();

		ta.mark(TS::start);
		// preprocess the image
		ocr::preprocess(im);
		api->SetImage(im.data, im.cols, im.rows, 1, im.step);

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

		// postprocess
		ocr::drawBoxes(im, *rects, cfg);
		ta.mark(TS::end);

		// log time data
		times.emplace_back(ta.duration(TS::end));
	}

	// show results
	auto sum = ocr::stableSum(times);
	std::cout << '\n'
			  << "n tests: " << ocr::nIter << '\n'
			  << "total test time: " << sum << '\n'
			  << "avg. processing time: "
			  << sum / static_cast<double>(ocr::nIter) << "ms"
			  << '\n';

	return 0;
}
*/
// ************************************************************************* //
