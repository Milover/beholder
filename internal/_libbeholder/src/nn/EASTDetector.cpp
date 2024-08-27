/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <array>
#include <filesystem>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>

#include "EASTDetector.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

std::array<double, 3> EASTDetector::getMean(const cv::Mat& img) const
{
	if (useHardCodedMean)
	{
		// for more info, see:
		// https://docs.opencv.org/4.10.0/d4/d43/tutorial_dnn_text_spotting.html
		return std::array<double, 3> {123.68, 116.78, 103.94};
	}
	return Base::getMean(img);
}

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

EASTDetector::EASTDetector()
{
	swapRB_ = true;	// XXX: dunno why, but that's how the examples set it
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool EASTDetector::init()
{
	using EAST = cv::dnn::TextDetectionModel_EAST;

	// reset now in case something throws, since we're doing no input checking
	resetPtr();

	// XXX: no checks, we assume that it's been checked and is correct; yolo
	// should be a '.pb' file though
	std::filesystem::path modelFile
	{
		std::filesystem::path{modelPath} / model
	};

	EAST* p {new EAST {modelFile.string()}};
	if (p)
	{
		p->setConfidenceThreshold(confidenceThreshold);
		p->setNMSThreshold(nmsThreshold);
	}
	resetPtr(static_cast<Base::TDM*>(p));

	return hasPtr();
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
