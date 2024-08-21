/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <filesystem>
#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>

#include "Detector.h"
#include "Processor.h"
#include "RawImage.h"
#include "Rectangle.h"
#include "Result.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

Detector::Detector()
{}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Detector::~Detector()
{
	if (m_)
	{
		delete m_;
		m_ = nullptr;
	}
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Detector::clear()
{
	res_.clear();
}

bool Detector::detect(const RawImage& raw)
{
	res_.clear();
	if (!m_)
	{
		return false;
	}
	auto img {rawToMatPtr(raw)};
	if (!img)
	{
		return false;
	}

	m_->setInputParams
	(
		scale_,
		cv::Size {img->cols, img->rows},
		cv::mean(*img),	// XXX: not sure about this guy
		swapRB_
	);
	// OPTIMIZE: should we reserve here?
	std::vector<cv::RotatedRect> rects;
	std::vector<float> confidences;
	m_->detectTextRectangles(*img, rects, confidences);	// EAST can't do detect(...)

	res_.reserve(rects.size());
	for (auto i {0ul}; i < rects.size(); ++i)
	{
		// FIXME: we're throwing away orientation data, should do a perspective
		// transform or something
		cv::Rect r {rects[i].boundingRect()};
		res_.emplace_back
		(
			Result
			{
				std::string {},
				Rectangle {r.x, r.y, r.x + r.width, r.y + r.height},
				static_cast<double>(confidences[i])
			}
		);
	}
	return !res_.empty();
}

const std::vector<Result>& Detector::getResults() const
{
	return res_;
}

bool Detector::init()
{
	using DB = cv::dnn::TextDetectionModel_DB;
	using EAST = cv::dnn::TextDetectionModel_EAST;

	if (m_)
	{
		delete m_;
		m_ = nullptr;
	}

	std::filesystem::path modelFile
	{
		std::filesystem::path{modelPath} / model
	};
	auto ext {modelFile.extension()};

	// TODO: should probably use cv::dnn::readNet..., so we don't have to
	// fuck around with temporary files, and it clears up which detector
	// class should be constructed.
	// TODO: all of this extension checking stuff should be done from Go.
	// TODO: make the setup adjustable
	if (ext == ".onnx")
	{
		m_ = static_cast<TDN*>(new DB {modelFile.string()});
		if (!m_)
		{
			return false;
		}

		DB* m {static_cast<DB*>(m_)};
		m->setBinaryThreshold(0.3);
		m->setPolygonThreshold(0.5);
		m->setMaxCandidates(200);
		m->setUnclipRatio(2.0);

		scale_ = 1.0/255.0;	// XXX: ?
	}
	else if (ext == ".pb")
	{
		m_ = static_cast<TDN*>(new EAST {modelFile.string()});
		if (!m_)
		{
			return false;
		}

		EAST* m {static_cast<EAST*>(m_)};
		m->setConfidenceThreshold(0.5);
		m->setNMSThreshold(0.4);

		scale_ = 1.0;	// XXX: ?
		swapRB_ = true;	// XXX: required?
	}
	return static_cast<bool>(m_);
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
