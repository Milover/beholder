/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "PARSeqDetector.h"
#include "Processor.h"
#include "Ops.h"
#include "Utility.h"

#include "Test.h"

// * * * * * * * * * * * * * * * * Constants * * * * * * * * * * * * * * * * //

namespace beholder
{

namespace test
{

// the internal/neural directory
const std::filesystem::path internalDir
{
	std::filesystem::path {__FILE__}.parent_path() / "../../../neural/"
};

using ImgLabel = std::pair<std::string, std::string>;

const std::vector tests
{
	ImgLabel{"testdata/images/parseq/text.png",			"TEXT"},
	ImgLabel{"testdata/images/parseq/chewbacca.jpg",	"CHEWBACCA"},
	ImgLabel{"testdata/images/parseq/chevrol.jpg",		"Chevrol"},
	ImgLabel{"testdata/images/parseq/salmon.jpg",		"SALMON"},
	ImgLabel{"testdata/images/parseq/3rd_ave.jpg",		"3rdAve"},
	ImgLabel{"testdata/images/parseq/kappa.png",		"Kappa"},
};

} // end namespace test
} // end namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// chdir to the go project internal/neural directory to make our life easier
	std::filesystem::current_path(beholder::test::internalDir);

	// NOTE: seems to be performing better than the DB detector
	beholder::PARSeqDetector det {};
	det.modelPath = "model/_internal/parseq";
	det.model = "parseq-128x32px.onnx";
	det.size = beholder::PARSeqDetector::Vec2<> {128, 32};
	if (!det.init())
	{
		std::cerr << "Could not initialize PARSeq text detector.\n";
		return 1;
	}

	// setup image processor
	beholder::Processor ip {};

	bool pass {true};
	for (const auto& [img, label] : beholder::test::tests)
	{
		// read/load an image
		ip.readImage(img, cv::IMREAD_COLOR);

		// run text recognition
		if (!det.detect(ip.getRawImage()))
		{
			pass = false;
			std::cerr << "Could not recognize text with PARSeq detector for image: "
					  << img << std::endl;
			continue;
		}
		if (det.getResults().front().text != label)
		{
			pass = false;
		}
		std::cout << "expected: " << std::quoted(label) << '\t'
				  << std::fixed << std::setprecision(3)
				  << "got: (conf: " << det.getResults().front().confidence << ") "
				  << std::quoted(det.getResults().front().text)
				  << std::endl;
	}
	return pass ? 0 : 1;
}

// ************************************************************************* //
