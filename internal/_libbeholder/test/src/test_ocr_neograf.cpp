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

// the internal/neural directory
const std::filesystem::path internalDir
{
	std::filesystem::path {__FILE__}.parent_path() / "../../../neural/"
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

const std::vector<std::string> expected
{
	"V20000229"
};

} // end namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// chdir to the go project internal/neural directory to make our life easier
	std::filesystem::current_path(beholder::internalDir);

	// setup tesseract
	beholder::Tesseract t {};
	//t.configPaths = std::vector<std::string>{"testdata/configs/test_neograf.patterns.config"};
	t.modelPath = "model/_internal/tesseract/dotmatrix";
	t.model = "dotOCRDData1";
	t.pageSegMode = 7;
	t.variables = std::vector<std::pair<std::string, std::string>>
	{
		{"load_system_dawg", "0"},
		{"load_freq_dawg", "0"},
		{"tessedit_char_whitelist", "V0123456789"},
		{"user_patterns_file", "testdata/configs/test_neograf.patterns"}
	};
	if (!t.init())
	{
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}
	//t.dumpVariables();
	std::cout << "No dawgs: " << t.getNoDawgs() << '\n';

	// setup image processor
	beholder::Processor ip {};
	// preprocessin
	ip.preprocessing.emplace_back(new beholder::AutoCrop {35, 50.0, 50.0, 10.0});
	ip.preprocessing.emplace_back(new beholder::Resize {205, 34});
	ip.preprocessing.emplace_back(new beholder::NormalizeBrightnessContrast {1.5});
	ip.preprocessing.emplace_back(new beholder::Threshold {0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU});
	ip.preprocessing.emplace_back(new beholder::GaussianBlur {3, 5, 0, 0});
	ip.preprocessing.emplace_back(new beholder::EqualizeHistogram {});
	ip.preprocessing.emplace_back(new beholder::Morphology {cv::MORPH_RECT, 3, 3, cv::MORPH_DILATE, 1});
	// postprocessing
	ip.postprocessing.emplace_back(new beholder::DrawBoundingBoxes {std::array<float, 4>{0, 0, 0, 0}, 3});

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

	// NOTE: the analysis (seemingly) isn't being re-run
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
