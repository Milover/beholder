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
#include "Tesseract.h"
#include "Utility.h"
#include "YOLOv8Detector.h"

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
	"testdata/images/tags_0.bmp"
	//"testdata/images/tags_2.jpeg"
};

} // end namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	auto pwd {std::filesystem::current_path()};	// so we can return
	// chdir to the go project internal/neural directory to make our life easier
	std::filesystem::current_path(beholder::internalDir);

	// set up yolo
	beholder::YOLOv8Detector yolo {};
	yolo.modelPath = "model/_internal/yolo";
	yolo.model = "fima_v8n_640-50e-b16-1280px.onnx";
	yolo.size = beholder::YOLOv8Detector::Vec2<> {1280, 1280};
	if (!yolo.init())
	{
		std::cerr << "Could not initialize yolo.\n";
		return 1;
	}

	// set up east
	beholder::EASTDetector east {};
	east.modelPath = "model/_internal/east";
	east.model = "east.pb";
	east.size = beholder::EASTDetector::Vec2<> {320, 320};
	if (!east.init())
	{
		std::cerr << "Could not initialize east.\n";
		return 1;
	}

	// set up tesseract
	beholder::Tesseract tess {};
	tess.modelPath = "model/_internal/tesseract/dflt";
	tess.model = "eng";
	tess.pageSegMode = 8;	// word, or 7 - line
	tess.variables = std::vector<std::pair<std::string, std::string>>
	{
		{"load_system_dawg", "0"},
		{"load_freq_dawg", "0"},
		{"tessedit_char_whitelist", "ABCDEFGHIJKLMNOPRSTUVZ-0123456789"},
		{"user_patterns_file", "testdata/configs/test_neograf.patterns"}
	};
	if (!tess.init())
	{
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}

	// setup image processor
	beholder::Processor ip {};
	// preprocessing
	ip.preprocessing.emplace_back(new beholder::Rescale {2.0});
	ip.preprocessing.emplace_back(new beholder::Grayscale {});
	ip.preprocessing.emplace_back(new beholder::NormalizeBrightnessContrast {0.0, 0.5});
	ip.preprocessing.emplace_back(new beholder::GaussianBlur {5, 5, 0, 0});
	ip.preprocessing.emplace_back(new beholder::Threshold {0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU});
	ip.preprocessing.emplace_back(new beholder::AddPadding {10, 0.0});
//	ip.preprocessing.emplace_back(new beholder::Invert {});
//	ip.preprocessing.emplace_back(new beholder::AddPadding {10, 0.0});
//	ip.preprocessing.emplace_back(new beholder::Morphology {cv::MORPH_RECT, 3, 3, cv::MORPH_CLOSE, 1});
//	ip.preprocessing.emplace_back(new beholder::Morphology {cv::MORPH_RECT, 3, 3, cv::MORPH_OPEN, 1});
	ip.preprocessing.emplace_back(new beholder::Invert {});
//	ip.preprocessing.emplace_back(new beholder::AutoCrop {20, 20.0, 10.0, 10.0f, 255.0});
	ip.preprocessing.emplace_back(new beholder::Rescale {0.5});
	// postprocessing
	ip.postprocessing.emplace_back(new beholder::DrawBoundingBoxes {std::array<float, 4>{0, 255, 0, 0}, 2});
	ip.postprocessing.emplace_back(new beholder::DrawLabels {std::array<float, 4>{0, 255, 0, 0}, 1.5, 2});

	// read/load an image
	ip.readImage(beholder::testImage, cv::IMREAD_COLOR);
	std::filesystem::current_path(pwd);	// return to original directory

	std::vector<beholder::Result> yoloResults;
	std::vector<beholder::Result> eastResults;

	// yolo
	if (!yolo.detect(ip.getRawImage()))
	{
		std::cerr << "yolo error\n";
		return 1;
	}
	for (const auto& r : yolo.getResults())
	{
		yoloResults.emplace_back
		(
			r.text,
			beholder::Rectangle
			{
				r.box.cRef().left - 15,
				r.box.cRef().top - 15,
				r.box.cRef().right + 15,
				r.box.cRef().bottom + 15
			},
			r.boxRotAngle,
			r.confidence
		);
	}

	// east
	for (const auto& r : yoloResults)
	{
		ip.setROI(r.box);
		if (!east.detect(ip.getRawImage()))
		{
			std::cerr << "east error\n";
		}
		for (const auto& er : east.getResults())
		{
			eastResults.emplace_back
			(
				er.text,
				beholder::Rectangle
				{
					cvFloor(static_cast<float>(1.01 * er.box.cRef().left + r.box.cRef().left)),
					cvFloor(static_cast<float>(1.01 * er.box.cRef().top + r.box.cRef().top)),
					cvFloor(static_cast<float>(1.01 * er.box.cRef().right + r.box.cRef().left)),
					cvFloor(static_cast<float>(1.01 * er.box.cRef().bottom + r.box.cRef().top))
				},
				er.boxRotAngle,
				er.confidence
			);
		}
		ip.resetROI();
	}

	// OCR
	auto count {0ul};
	for (const auto& r : eastResults)
	{
		ip.setRotatedROI(r.box, r.boxRotAngle);
		ip.preprocess();
		ip.writeImage("tmp_" + std::to_string(count) + ".jpeg");
		if (!(tess.setImage(ip.getRawImage()) && tess.recognizeText()))
		{
			std::cerr << "tesseract error\n";
		}
		for (const auto& tr : tess.getResults())
		{
			std::cout << tr.text << '\n';
		}
		ip.resetROI();
		++count;
	}

	// postprocess
	//ip.postprocess(yoloResults);
	//ip.writeImage("yolo.jpeg");

	ip.postprocess(eastResults);
	ip.writeImage("east.jpeg");

	ip.writeImage("tess.jpeg");

	return 0;
}

// ************************************************************************* //
