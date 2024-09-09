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

#include "YOLOv8Detector.h"
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
	//"testdata/images/ultralytics_bus.jpg"
	//"testdata/images/tags_0.bmp"
	//"testdata/images/tags_1.jpeg"
	//"testdata/images/sawlog_0.png"
	//"testdata/images/sawlog_1.png"
	"testdata/images/sawlog_2.png"
};

const std::vector<std::string> cocoClasses
{
	"person",
	"bicycle",
	"car",
	"motorcycle",
	"airplane",
	"bus",
	"train",
	"truck",
	"boat",
	"traffic light",
	"fire hydrant",
	"stop sign",
	"parking meter",
	"bench",
	"bird",
	"cat",
	"dog",
	"horse",
	"sheep",
	"cow",
	"elephant",
	"bear",
	"zebra",
	"giraffe",
	"backpack",
	"umbrella",
	"handbag",
	"tie",
	"suitcase",
	"frisbee",
	"skis",
	"snowboard",
	"sports ball",
	"kite",
	"baseball bat",
	"baseball glove",
	"skateboard",
	"surfboard",
	"tennis racket",
	"bottle",
	"wine glass",
	"cup",
	"fork",
	"knife",
	"spoon",
	"bowl",
	"banana",
	"apple",
	"sandwich",
	"orange",
	"broccoli",
	"carrot",
	"hot dog",
	"pizza",
	"donut",
	"cake",
	"chair",
	"couch",
	"potted plant",
	"bed",
	"dining table",
	"toilet",
	"tv",
	"laptop",
	"mouse",
	"remote",
	"keyboard",
	"cell phone",
	"microwave",
	"oven",
	"toaster",
	"sink",
	"refrigerator",
	"book",
	"clock",
	"vase",
	"scissors",
	"teddy bear",
	"hair drier",
	"toothbrush"
};

const std::vector<std::string> fimaClasses
{
	"tag"
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

	// setup object detector
	beholder::YOLOv8Detector det {};
	det.modelPath = "model/_internal/yolo";
	det.model = "fima_v8n_640-50e-b16-1280px.onnx";
	det.classes = beholder::fimaClasses;
	det.size = beholder::YOLOv8Detector::Vec2<> {1280, 1280};
	if (!det.init())
	{
		std::cerr << "Could not initialize object detector.\n";
		return 1;
	}

	// setup image processor
	beholder::Processor ip {};
	// preprocessing
	// WARNING: images must be rescaled to multiples of 32px
	//ip.preprocessing.emplace_back(new beholder::Crop {0, 0, 2560, 2560});
	// postprocessing
	ip.postprocessing.emplace_back(new beholder::DrawBoundingBoxes {std::array<float, 4>{0, 255, 0, 0}, 2});
	ip.postprocessing.emplace_back(new beholder::DrawLabels {std::array<float, 4>{0, 255, 0, 0}, 0.65, 2});

	// read/load an image
	ip.readImage(beholder::testImage, cv::IMREAD_COLOR);

	// preprocess the image
	ip.preprocess();

	// run text detection
	if (!det.detect(ip.getRawImage()))
	{
		std::cerr << "Could not detect objects.\n";
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
