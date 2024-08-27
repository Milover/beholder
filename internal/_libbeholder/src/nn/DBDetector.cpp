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

#include "DBDetector.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

std::array<double, 3> DBDetector::getMean(const cv::Mat& img) const
{
	if (useHardCodedMean)
	{
		// for more info, see:
		// https://docs.opencv.org/4.10.0/d4/d43/tutorial_dnn_text_spotting.html
		return std::array<double, 3> {122.67891434, 116.66876762, 104.00698793};
	}
	return Base::getMean(img);
}

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

DBDetector::DBDetector()
{
	scale_ = 1.0 / 255.0;	// XXX: dunno why, but that's how the examples set it
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool DBDetector::init()
{
	using DB = cv::dnn::TextDetectionModel_DB;

	// reset now in case something throws, since we're doing no input checking
	resetPtr();

	// XXX: no checks, we assume that it's been checked and is correct; yolo
	// should be a '.onnx' file though
	std::filesystem::path modelFile
	{
		std::filesystem::path{modelPath} / model
	};

	DB* p {new DB {modelFile.string()}};
	if (p)
	{
		p->setBinaryThreshold(binaryThreshold);
		p->setPolygonThreshold(polygonThreshold);
		p->setMaxCandidates(maxCandidates);
		p->setUnclipRatio(unclipRatio);
	}
	resetPtr(static_cast<Base::TDM*>(p));

	return hasPtr();
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
