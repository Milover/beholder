/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "ConversionInfo.h"
#include "Processor.h"
#include "RawImage.h"
#include "Result.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

Processor::Processor()
:
	img_ {new cv::Mat{}}
{}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Processor::~Processor()
{
	delete img_;
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool Processor::decodeImage(void* buffer, int bufSize, int flags)
{
	*img_ = cv::Mat{1, bufSize, CV_8UC1, buffer};
	cv::imdecode(*img_, flags, img_);	// yolo
	return img_->data != NULL;
}

const cv::Mat& Processor::getImage() const
{
	return *img_;
}

std::size_t Processor::getImageID() const
{
	return id_;
}

RawImage Processor::getRawImage() const
{
	// WARNING: we assume that we can only have 8-bit Mono or BGR images
	return RawImage
	{
		id_,
		img_->rows,
		img_->cols,
		img_->elemSize() == 1ul ? static_cast<std::int64_t>(PxType::Mono8)
							  : static_cast<std::int64_t>(PxType::BGR8packed),
		static_cast<void*>(img_->data),
		img_->step1()
	};
}

bool Processor::postprocess(const std::vector<Result>& res)
{
	for (const auto& o : postprocessing)
	{
		// FIXME: should ask weather to overwrite or use a new output image
		if (!o->operator()(*img_, *img_, res))
		{
			// FIXME: should give info on what failed
			return false;
		}
	}
	return true;
}

bool Processor::preprocess()
{
	for (const auto& o : preprocessing)
	{
		// FIXME: should ask weather to overwrite or use a new output image
		if (!o->operator()(*img_, *img_))
		{
			// FIXME: should give info on what failed
			return false;
		}
	}
	return true;
}

bool Processor::receiveRawImage(const RawImage& raw)
{
	id_ = raw.id;

	auto info {getConversionInfo(static_cast<PxType>(raw.pixelType))};
	if (!info)
	{
		std::cerr << "could not get conversion info (ID: " << id_ << "): "
				  << "unknown pixel type: " << raw.pixelType << std::endl;
		return false;
	}
	cv::Mat tmp
	{
		raw.rows,
		raw.cols,
		info->inputType,
		raw.buffer,
		raw.step > 0ul ? raw.step : static_cast<std::size_t>(cv::Mat::AUTO_STEP)
	};

	// convert the color scheme if necessary
	if (info->colorConvCode == -1)
	{
		tmp.copyTo(*img_);
	}
	else
	{
		cv::cvtColor(tmp, *img_, info->colorConvCode, info->outChannels);
	}
	return true;
}

bool Processor::readImage(const std::string& path, int flags)
{
	*img_ = cv::imread(path, flags);
	return img_->data != NULL;
}

void Processor::showImage(const std::string& title) const
{
	cv::imshow(title, *img_);
	cv::waitKey();
}

bool Processor::writeImage(const std::string& filename) const
{
	std::vector<int> flags
	{
		cv::IMWRITE_PNG_COMPRESSION, 0,				// lowest compression level
		cv::IMWRITE_JPEG2000_COMPRESSION_X1000, 0	// lowest compression level
	};
	return cv::imwrite(filename, *img_, flags);
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

std::unique_ptr<cv::Mat> rawToMatPtr(const RawImage& raw)
{
	auto info {getConversionInfo(static_cast<PxType>(raw.pixelType))};
	if (!info)
	{
		std::cerr << "could not get conversion info (ID: " << raw.id << "): "
				  << "unknown pixel type: " << raw.pixelType << std::endl;
		return nullptr;
	}
	return std::unique_ptr<cv::Mat>
	{
		new cv::Mat
		{
			raw.rows,
			raw.cols,
			info->inputType,
			raw.buffer,
			raw.step > 0ul ? raw.step : static_cast<std::size_t>(cv::Mat::AUTO_STEP)
		}
	};
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
