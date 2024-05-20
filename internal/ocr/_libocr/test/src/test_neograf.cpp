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
	"testdata/images/neograf/imagefile_1.bmp"
	//"testdata/images/neograf/imagefile_8.bmp"
	//"testdata/images/neograf/imagefile_13.bmp"
	//"testdata/images/neograf/imagefile_14.bmp"
	//"testdata/images/neograf/imagefile_15.bmp"
	//"testdata/images/neograf/imagefile_18.bmp"
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
	t.configPaths = std::vector<std::string>{"testdata/configs/test_neograf.patterns.config"};
	t.modelPath = "_models/dotmatrix";
	t.model = "dotOCRDData1";
	t.pageSegMode = 7;
	t.variables = std::vector<std::pair<std::string, std::string>>
	{
		{"load_system_dawg", "0"},
		{"load_freq_dawg", "0"},
		{"tessedit_char_whitelist", "V0123456789"}
	};
	if (!t.init())
	{
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}

	// setup image processor
	ocr::ImageProcessor ip {};
	// preprocessin
	ip.preprocessing.emplace_back(new ocr::AutoCrop {35, 50.0, 50.0, 10.0});
	ip.preprocessing.emplace_back(new ocr::Resize {205, 34});
	ip.preprocessing.emplace_back(new ocr::NormalizeBrightnessContrast {1.5});
	ip.preprocessing.emplace_back(new ocr::Threshold {0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU});
	ip.preprocessing.emplace_back(new ocr::GaussianBlur {3, 5, 0, 0});
	ip.preprocessing.emplace_back(new ocr::EqualizeHistogram {});
	ip.preprocessing.emplace_back(new ocr::Morphology {cv::MORPH_RECT, 3, 3, cv::MORPH_DILATE, 1});
	// postprocessing
	ip.postprocessing.emplace_back(new ocr::DrawTextBoxes {std::array<float, 4>{0, 0, 0, 0}, 3});

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
