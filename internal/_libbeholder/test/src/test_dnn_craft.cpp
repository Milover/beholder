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

#include "CRAFTDetector.h"
#include "Processor.h"
#include "Ops.h"
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
	//"testdata/images/neograf/imagefile_1.bmp"
	//"testdata/images/sawlog_0.png"
	//"testdata/images/sawlog_0.cropped.png"
	//"testdata/images/tags_0.bmp"
	//"testdata/images/text_2_line_1_word.png"
	//"testdata/images/text_1_line_2_word.png"
	"testdata/images/fima/tags_10.png"
	//"testdata/images/fima/tags_0.bmp"
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
	// chdir to the go project internal/neural directory to make our life easier
	std::filesystem::current_path(beholder::internalDir);

	// NOTE: seems to be performing better than the DB detector
	beholder::CRAFTDetector det {};
	det.backend = beholder::BackendCUDA;
	det.target = beholder::TargetCUDA;
	det.modelPath = "model/_internal/craft";
	det.model = "craft-320px.onnx";
	det.size = beholder::CRAFTDetector::Vec2<> {320, 320};
	if (!det.init())
	{
		std::cerr << "Could not initialize CRAFT text detector.\n";
		return 1;
	}

	// setup image processor
	beholder::Processor ip {};
	// preprocessing
	// WARNING: images must be rescaled to multiples of 80px
	//ip.preprocessing.emplace_back(new beholder::Resize {160, 160});
	//ip.preprocessing.emplace_back(new beholder::NormalizeBrightnessContrast {0.05, 0.05});
	//ip.preprocessing.emplace_back(new beholder::CorrectGamma {0.75});
	// postprocessing
	ip.postprocessing.emplace_back
	(
		new beholder::DrawBoundingBoxes {std::array<float, 4>{0, 255, 0, 0}, 2}
	);

	// read/load an image
	ip.readImage(beholder::testImage, cv::IMREAD_COLOR);

	// preprocess the image
	ip.preprocess();

	// run text detection
	if (!det.detect(ip.getRawImage()))
	{
		std::cerr << "Could not detect text with CRAFT detector.\n";
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
