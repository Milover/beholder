/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <filesystem>
#include <iostream>
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

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

std::array<double, 3> Detector::getMean(const cv::Mat& img) const
{
	cv::Scalar s {cv::mean(img)};
	return std::array<double, 3> {s[0], s[1], s[2]};
}

bool Detector::hasPtr() const
{
	return static_cast<bool>(p_);
}

void Detector::resetPtr(Detector::TDM* p)
{
	if (p_)
	{
		delete p_;
	}
	p_ = p;
}

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Detector::~Detector()
{
	resetPtr();
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Detector::clear()
{
	res_.clear();
}

bool Detector::detect(const RawImage& raw)
{
	res_.clear();
	if (!p_)
	{
		return false;
	}
	auto img {rawToMatPtr(raw)};
	if (!img)
	{
		return false;
	}
	if (img->cols % dimLCD != 0 || img->rows % dimLCD != 0)
	{
		std::cerr << "image dimensions not multiples of "
				  << dimLCD << "px" << std::endl;
		return false;
	}

	// set params
	auto mean {getMean(*img)};
	p_->setInputParams
	(
		scale_,
		cv::Size {img->cols, img->rows},
		cv::Scalar {mean[0], mean[1], mean[2]},
		swapRB_
	);
	std::vector<cv::RotatedRect> rects;	// OPTIMIZE: should we reserve here?
	std::vector<float> confidences;		// OPTIMIZE: should we reserve here?

	// run detection
	p_->detectTextRectangles(*img, rects, confidences);	// EAST can't do detect(...)

	// store results
	res_.reserve(10);	// guesstimate
	for (auto i {0ul}; i < rects.size(); ++i)
	{
		// FIXME: we're throwing away orientation data, we should instead
		// do a perspective transform or store a quadrangle or something
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

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
