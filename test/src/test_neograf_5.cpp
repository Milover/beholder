/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <iostream>
#include <string>

#include "libocr.h"

#include "Test.h"

// * * * * * * * * * * * * * * * * Constants * * * * * * * * * * * * * * * * //

namespace ocr
{

const std::string config
{R"(
{
	"config_paths": ["../testdata/configs/test_neograf_5.patterns.config"],
	"model_path": "../../assets/tessdata/dotmatrix",
	"model": "dotOCRDData1"
}
)"};


const std::string testImage
{
	"../testdata/neograf/imagefile_5.bmp"
};

const std::string expected
{
	"V20000229"
};

} // end namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// setup the config
	ocr::Config cfg {};
	if (!cfg.parse(ocr::config))
	{
		std::cerr << "Could not parse JSON.\n";
		return 1;
	}

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
	ip.preprocess(cfg);
	t.setImage(ip);

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
