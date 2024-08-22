/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <filesystem>
#include <iostream>
#include <string>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "Processor.h"
#include "Ops.h"
#include "Tesseract.h"
#include "Utility.h"

#include "Test.h"

// * * * * * * * * * * * * * * * * Constants * * * * * * * * * * * * * * * * //

namespace beholder
{

// the internal/ocr directory
const std::filesystem::path internalDir
{
	std::filesystem::path {__FILE__}.parent_path() / "../../../ocr/"
};

const std::string testImage
{
	"testdata/images/dukat/imagefile_145.jpeg"
};

const std::vector<std::string> expected
{
	"15.11.2023.",
	"11:57"
};

} // end namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// chdir to the go project internal/ocr directory to make our life easier
	std::filesystem::current_path(beholder::internalDir);

	// setup tesseract
	beholder::Tesseract t {};
	//t.configPaths = std::vector<std::string>{"testdata/configs/test_dukat.patterns.config"};
	//t.modelPath = "_models/best";
	t.modelPath = "model/dotmatrix";
	//t.model = "5x5_Dots_FT_500";
	//t.model = "DisplayDots_FT_500";
	//t.model = "DotMatrix_FT_500";
	//t.model = "Dotrice_FT_500";
	//t.model = "LCDDot_FT_500";	// 151 failed
	//t.model = "Orario_FT_500";
	t.model = "Transit_FT_500";	// 146 failed
	//t.model = "dotOCRDData1";
	//t.model = "eng";	// 161 failed
	t.pageSegMode = 6;
	t.variables = std::vector<std::pair<std::string, std::string>>
	{
		{"load_system_dawg", "0"},
		{"load_freq_dawg", "0"},
		{"classify_bln_numeric_mode", "1"},
		{"tessedit_char_whitelist", ".:0123456789"},
		{"user_patterns_file", "testdata/configs/test_dukat.patterns"}
	};
	if (!t.init())
	{
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}

	// setup image processor
	beholder::Processor ip {};
	// preprocessin
	ip.preprocessing.emplace_back(new beholder::Crop {700, 225, 650, 800});
	//ip.preprocessing.emplace_back(new beholder::Threshold {71, 255, cv::THRESH_TOZERO});
	ip.preprocessing.emplace_back(new beholder::Invert {});
	ip.preprocessing.emplace_back(new beholder::MedianBlur {7});
	//ip.preprocessing.emplace_back(new beholder::EqualizeHistogram {});
	//ip.preprocessing.emplace_back(new beholder::CLAHE {120, 12, 12});
	ip.preprocessing.emplace_back(new beholder::AutoCrop {50, 50.0, 50.0, 15.0});
	ip.preprocessing.emplace_back(new beholder::Resize {225, 90});
	ip.preprocessing.emplace_back(new beholder::Rotate {180});
	ip.preprocessing.emplace_back(new beholder::DivGaussianBlur {255.0, 5.0, 5.0, 0, 0});
	//ip.preprocessing.emplace_back(new beholder::DivGaussianBlur {255.0, 5.0, 5.0, 0, 0});
	//ip.preprocessing.emplace_back(new beholder::Morphology {cv::MORPH_RECT, 3, 3, cv::MORPH_DILATE, 1});
	//ip.preprocessing.emplace_back(new beholder::GaussianBlur {0, 0, 13, 13});
	//ip.preprocessing.emplace_back(new beholder::NormalizeBrightnessContrast {5.0});
	//ip.preprocessing.emplace_back(new beholder::GaussianBlur {3, 5, 30, 30});
	//ip.preprocessing.emplace_back(new beholder::MedianBlur {3});
	//ip.preprocessing.emplace_back(new beholder::AutoCrop {15, 20.0, 10.0, 10.0});
	//ip.preprocessing.emplace_back(new beholder::GaussianBlur {3, 5, 0, 0});
	//ip.preprocessing.emplace_back(new beholder::EqualizeHistogram {});
	//ip.preprocessing.emplace_back(new beholder::CLAHE {10, 1, 4});
	//ip.preprocessing.emplace_back(new beholder::GaussianBlur {3, 5, 0, 0});
	//ip.preprocessing.emplace_back(new beholder::EqualizeHistogram {});
	//ip.preprocessing.emplace_back(new beholder::CLAHE {10, 1, 4});
	ip.preprocessing.emplace_back(new beholder::Threshold {0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU});
	//ip.preprocessing.emplace_back(new beholder::Threshold {120, 255, cv::THRESH_BINARY});
	ip.preprocessing.emplace_back(new beholder::AutoCrop {15, 20.0, 10.0, 10.0});
	ip.preprocessing.emplace_back(new beholder::GaussianBlur {3, 5, 0, 0});
	//ip.preprocessing.emplace_back(new beholder::Resize {225, 75});
	//ip.preprocessing.emplace_back(new beholder::Morphology {cv::MORPH_RECT, 2, 3, cv::MORPH_OPEN, 1});
	//ip.preprocessing.emplace_back(new beholder::EqualizeHistogram {});
	ip.preprocessing.emplace_back(new beholder::Morphology {cv::MORPH_RECT, 4, 4, cv::MORPH_OPEN, 1});
	//ip.preprocessing.emplace_back(new beholder::Morphology {cv::MORPH_RECT, 3, 3, cv::MORPH_DILATE, 1});
	//ip.preprocessing.emplace_back(new beholder::Morphology {cv::MORPH_RECT, 3, 3, cv::MORPH_DILATE, 1});
	//ip.preprocessing.emplace_back(new beholder::Morphology {cv::MORPH_RECT, 2, 2, cv::MORPH_OPEN, 1});
	//ip.preprocessing.emplace_back(new beholder::AddPadding {10});
	//ip.preprocessing.emplace_back(new beholder::MedianBlur {5});
	//ip.preprocessing.emplace_back(new beholder::Threshold {0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU});
	//ip.preprocessing.emplace_back(new beholder::MedianBlur {9});
	//ip.preprocessing.emplace_back(new beholder::NormalizeBrightnessContrast {0.5});
	//ip.preprocessing.emplace_back(new beholder::CLAHE {40, 2, 2});
	//ip.preprocessing.emplace_back(new beholder::CLAHE {120, 12, 12});
	//ip.preprocessing.emplace_back(new beholder::Threshold {195, 255, cv::THRESH_BINARY});
	//ip.preprocessing.emplace_back(new beholder::Threshold {0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU});
	//ip.preprocessing.emplace_back(new beholder::GaussianBlur {3, 9, 0, 0});
	//ip.preprocessing.emplace_back(new beholder::Resize {225, 75});
	//ip.preprocessing.emplace_back(new beholder::Morphology {cv::MORPH_RECT, 2, 3, cv::MORPH_OPEN, 2});
	//ip.preprocessing.emplace_back(new beholder::MedianBlur {7});
	//ip.preprocessing.emplace_back(new beholder::NormalizeBrightnessContrast {0.5});
	//ip.preprocessing.emplace_back(new beholder::Crop {-10, 2000, -10, 900});	// a nop crop
	//ip.preprocessing.emplace_back(new beholder::Rotate {360});				// a nop rotate
	//ip.preprocessing.emplace_back(new beholder::MedianBlur {5});
	//ip.preprocessing.emplace_back(new beholder::Resize {195, 80});
	//ip.preprocessing.emplace_back(new beholder::NormalizeBrightnessContrast {1.5});
	//ip.preprocessing.emplace_back(new beholder::GaussianBlur {3, 5, 0, 0});
	//ip.preprocessing.emplace_back(new beholder::Threshold {0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU});
	//// postprocessing
	//ip.postprocessing.emplace_back(new beholder::DrawBoundingBoxes {std::array<float, 4>{0, 0, 0, 0}, 3});

	// read/load an image
	ip.readImage(beholder::testImage, cv::IMREAD_GRAYSCALE);

	// preprocess the image
	ip.preprocess();
	if (!t.setImage(ip.getRawImage()))
	{
		std::cerr << "Could not set image.\n";
		return 1;
	}
	//ip.writeImage();
	//ip.showImage();

	// run ocr
	if (!t.recognizeText())
	{
		std::cerr << "Could not detect/recognize text.\n";
		return 1;
	}

	// postprocess
	ip.postprocess(t.getResults());

	return beholder::checkOCRResults(t.getResults(), beholder::expected);
}

// ************************************************************************* //
