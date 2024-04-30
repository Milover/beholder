/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <iostream>
#include <string>

#include <opencv2/core/mat.hpp>

#include "Config.h"
#include "ImageProcessor.h"
#include "Tesseract.h"
#include "Utility.h"

#include "Test.h"

// * * * * * * * * * * * * * * * * Constants * * * * * * * * * * * * * * * * //

namespace ocr
{

static constexpr char const* patternConfig
{
	"/Users/philipp/Documents/c++/projects/libocr/test/testdata/configs/test_neograf_5.patterns.config"
};

static constexpr char const* testImage
{
	"/Users/philipp/Documents/c++/projects/libocr/test/testdata/neograf/imagefile_5.bmp"
};

static constexpr char const* modelName
{
	"dotOCRDData1"
};

static constexpr char const* expected
{
	"V20000229"
};

} // end namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// setup the config
	ocr::Config cfg {};
	cfg.configPaths.emplace_back(ocr::patternConfig);
	cfg.modelPath = ocr::tessdataDir;
	cfg.model = ocr::modelName;

	// setup tesseract
	ocr::Tesseract t {};
	if (!t.init(cfg))
	{
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}

	// setup image processor
	ocr::ImageProcessor ip {};

	// read/load an image
	ip.readImage(ocr::testImage);

	// preprocess the image
	ip.preprocess();
	t.setImage(ip.getImage());

	// NOTE: the analysis (seemingly) isn't being re-run
	// detect text (get text boxes)
	auto rects {t.detectText()};
	if (rects.empty())
	{
		// If text detection fails, text recognition will also fail, so it's
		// fine to exit right away.
		std::cerr << "Could not analyze text.\n";
		return 1;
	}
	// run ocr
	std::string text {t.recognizeTextStr()};
	ocr::trimWhiteLR(text);

	// postprocess
	ip.drawRectangles(rects, cfg);

	// show results
	std::string expectedStr {ocr::expected};
	std::cout << "Expected output:  " << expectedStr << '\n';
	std::cout << "OCR output:       " << text << '\n';
	//ip.showImage(im);

	if (expectedStr != text)
	{
		return 1;
	}
	return 0;
}

// ************************************************************************* //
