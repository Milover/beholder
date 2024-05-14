/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <filesystem>
#include <iostream>
#include <string>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "ImageProcessor.h"
#include "Ops.h"
#include "Tesseract.h"
#include "Utility.h"

#include "Test.h"

// * * * * * * * * * * * * * * * * Constants * * * * * * * * * * * * * * * * //

namespace ocr
{

// the internal/ocr directory
const std::filesystem::path internalDir
{
	"../../../"
};

const std::string testImage
{
	"testdata/images/neograf/imagefile_4.bmp"
};

const std::string expected
{
	"V20000229"
};

} // end namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// chdir to the go project internal/ocr directory to make our life easier
	std::filesystem::current_path(ocr::internalDir);

	// setup tesseract
	ocr::Tesseract t {};
	t.configPaths = std::vector<std::string>{"testdata/configs/test_neograf_5.patterns.config"};
	t.modelPath = "_models/dotmatrix";
	t.model = "dotOCRDData1";
	if (!t.init())
	{
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}

	// setup image processor
	ocr::ImageProcessor ip {};
	// preprocessin
	ip.preprocessing.emplace_back(new ocr::AutoCrop {50, 50.0, 50.0, 10.0});
	//ip.preprocessing.emplace_back(new ocr::Crop {-10, 2000, -10, 900});	// a nop crop
	//ip.preprocessing.emplace_back(new ocr::Rotate {360});				// a nop rotate
	ip.preprocessing.emplace_back(new ocr::Resize {205, 34});
	ip.preprocessing.emplace_back(new ocr::NormalizeBrightnessContrast {0.5});
	ip.preprocessing.emplace_back(new ocr::MedianBlur {3});
	ip.preprocessing.emplace_back(new ocr::Threshold {0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU});
	ip.preprocessing.emplace_back(new ocr::Morphology {cv::MORPH_RECT, 4, 4, cv::MORPH_OPEN, 1});
	//// postprocessing
	//ip.postprocessing.emplace_back(new ocr::DrawTextBoxes {std::array<float, 4>{0, 0, 0, 0}, 3});

	// read/load an image
	ip.readImage(ocr::testImage, cv::IMREAD_GRAYSCALE);

	// preprocess the image
	ip.preprocess();
	t.setImage(ip);
	//ip.writeImage();
	ip.showImage();

	// NOTE: the analysis (seemingly) isn't being re-run
	// run ocr
	if (!t.detectAndRecognize())
	{
		std::cerr << "Could not detect/recognize text.\n";
		return 1;
	}
	std::string txt {t.getResults().text};
	ocr::trimWhiteLR(txt);

	// postprocess
	ip.postprocess(t.getResults());

	// show results
	std::string expectedStr {ocr::expected};
	std::cout << "Expected output:  " << expectedStr << '\n';
	std::cout << "OCR output:       " << txt  << '\n';
	//ip.showImage();

	if (expectedStr != txt)
	{
		return 1;
	}
	return 0;
}

// ************************************************************************* //
