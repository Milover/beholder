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

#include "EASTDetector.h"
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
	//"testdata/images/sawlog_0.cropped.png"
	//"testdata/images/tags_0.bmp"
	//"testdata/images/sawlog_1.png"
	//"testdata/images/sawlog_2.png"
};

// TODO: 'expected' should be some kind of rect and we should check overlap
//const std::size_t expected {10};	// the number of results

} // end namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	auto pwd {std::filesystem::current_path()};	// so we can return
	// chdir to the go project internal/ocr directory to make our life easier
	std::filesystem::current_path(beholder::internalDir);

	// NOTE: seems to be performing better than the DB detector
	beholder::EASTDetector det {};
	det.modelPath = "model/east";
	det.model = "EAST.pb";
	det.size = 1920;
	if (!det.init())
	{
		std::cerr << "Could not initialize EAST text detector.\n";
		return 1;
	}

	// setup image processor
	beholder::Processor ip {};
	// preprocessing
	// WARNING: images must be rescaled to multiples of 80px
	//ip.preprocessing.emplace_back(new beholder::Crop {0, 0, 1920, 1920});
	// postprocessing
	ip.postprocessing.emplace_back(new beholder::DrawBoundingBoxes {std::array<float, 4>{0, 255, 0, 0}, 2});

	// read/load an image
	ip.readImage(beholder::testImage, cv::IMREAD_COLOR);

	// preprocess the image
	ip.preprocess();

	// run text detection
	if (!det.detect(ip.getRawImage()))
	{
		std::cerr << "Could not detect text with EAST detector.\n";
		return 1;
	}
	std::filesystem::current_path(pwd);	// return to original directory

	// postprocess
	ip.postprocess(det.getResults());
	ip.writeImage();
	//ip.showImage();

	return 0;
}

// ************************************************************************* //
