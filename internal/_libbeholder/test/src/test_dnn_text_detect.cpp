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

#include "DBDetector.h"
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
	//"testdata/images/sawlog_0.png"
	//"testdata/images/sawlog_0.cropped.png"
	"testdata/images/tags_0.bmp"
	//"testdata/images/sawlog_1.png"
	//"testdata/images/sawlog_2.png"
};

// TODO: 'expected' should be some kind of rect and we should check overlap
const std::size_t expected {10};	// the number of results

} // end namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// chdir to the go project internal/ocr directory to make our life easier
	std::filesystem::current_path(beholder::internalDir);

	// setup tesseract
	beholder::DBDetector db {};
	db.modelPath = "model/db";
	db.model = "DB_IC15_resnet50.onnx";
	//d.model = "DB_TD500_resnet50.onnx";
	if (!db.init())
	{
		std::cerr << "Could not initialize DB text detector.\n";
		return 1;
	}

	// NOTE: seems to be performing better than the DB detector
	beholder::EASTDetector east {};
	east.modelPath = "model/east";
	east.model = "EAST.pb";
	if (!east.init())
	{
		std::cerr << "Could not initialize EAST text detector.\n";
		return 1;
	}


	// setup image processor
	beholder::Processor ip {};
	// preprocessing
	// WARNING: images must be rescaled to multiples of 80px
	//ip.preprocessing.emplace_back(new beholder::Resize {1280, 736});
	//ip.preprocessing.emplace_back(new beholder::Resize {736, 736});
	//ip.preprocessing.emplace_back(new beholder::Resize {4000, 2880});
	ip.preprocessing.emplace_back(new beholder::Crop {0, 0, 1920, 1920});
	// postprocessing
	//ip.postprocessing.emplace_back(new beholder::DrawBoundingBoxes {std::array<float, 4>{0, 255, 0, 0}, 3});

	// read/load an image
	ip.readImage(beholder::testImage, cv::IMREAD_COLOR);

	// preprocess the image
	ip.preprocess();

	// run text detection
	if (!db.detect(ip.getRawImage()))
	{
		std::cerr << "Could not detect text with DB detector.\n";
		return 1;
	}
	if (!east.detect(ip.getRawImage()))
	{
		std::cerr << "Could not detect text with EAST detector.\n";
		return 1;
	}

	// postprocess
	//ip.postprocess(db.getResults());
	//ip.showImage();

	// print results
	std::cout << "Expected No. results:  " << beholder::expected << '\n'
			  << "DB output:             " << db.getResults().size() << '\n'
			  << "EAST output:           " << east.getResults().size() << '\n';
	bool ok
	{
		db.getResults().size() == beholder::expected
	 && east.getResults().size() == beholder::expected
	};
	return ok ? 0 : 1;
}

// ************************************************************************* //
