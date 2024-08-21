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

#include "Detector.h"
#include "Processor.h"
#include "Ops.h"
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
	//"testdata/images/neograf/imagefile_1.bmp"
	"testdata/images/sawlog_0.png"
	//"testdata/images/sawlog_1.png"
};

// TODO: 'expected' should be some kind of rect and we should check overlap

} // end namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// chdir to the go project internal/ocr directory to make our life easier
	std::filesystem::current_path(beholder::internalDir);

	// setup tesseract
	beholder::Detector d {};
	//t.configPaths = std::vector<std::string>{"testdata/configs/test_neograf.patterns.config"};
	// WARNING: images must be rescaled to multiples of 160px
	//d.modelPath = "model/db";
	//d.model = "DB_IC15_resnet50.onnx";
	//d.model = "DB_TD500_resnet50.onnx";
	d.modelPath = "model/east";
	d.model = "EAST.pb";	// seems to be performing better than the DB models
	if (!d.init())
	{
		std::cerr << "Could not initialize text detector.\n";
		return 1;
	}

	// setup image processor
	beholder::Processor ip {};
	// preprocessin
	//ip.preprocessing.emplace_back(new beholder::Resize {1280, 736});
	//ip.preprocessing.emplace_back(new beholder::Resize {736, 736});
	ip.preprocessing.emplace_back(new beholder::Resize {4000, 2880});
	// postprocessing
	ip.postprocessing.emplace_back(new beholder::DrawBoundingBoxes {std::array<float, 4>{0, 255, 0, 0}, 3});

	// read/load an image
	ip.readImage(beholder::testImage, cv::IMREAD_COLOR);

	// preprocess the image
	ip.preprocess();

	// NOTE: the analysis (seemingly) isn't being re-run
	// run ocr
	if (!d.detect(ip))
	{
		std::cerr << "Could not detect text.\n";
		return 1;
	}

	// postprocess
	ip.postprocess(d.getResults());

	ip.showImage();

	return 0;
}

// ************************************************************************* //
