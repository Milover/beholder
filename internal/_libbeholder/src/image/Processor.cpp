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
	img_ {new cv::Mat{}},
	roi_ {new cv::Mat{}}
{
	*roi_ = *img_;
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Processor::~Processor()
{}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool Processor::decodeImage(void* buffer, int bufSize, int flags)
{
	*img_ = cv::Mat{1, bufSize, CV_8UC1, buffer};
	cv::imdecode(*img_, flags, img_.get());	// yolo
	*roi_ = *img_;
	//return roi_->data != NULL;
	return roi_->data != nullptr;	// XXX: this should be ok
}

const cv::Mat& Processor::getImage() const
{
	return *roi_;
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
		roi_->rows,
		roi_->cols,
		roi_->elemSize() == 1ul ? static_cast<std::int64_t>(PxType::Mono8)
								: static_cast<std::int64_t>(PxType::BGR8packed),
		static_cast<void*>(roi_->data),
		roi_->step1(),
		roi_->elemSize() * 8	// bytes to bits
	};
}

bool Processor::postprocess(const std::vector<Result>& res)
{
	for (const auto& o : postprocessing)
	{
		// FIXME: should ask weather to overwrite or use a new output image
		if (!o->operator()(*roi_, *roi_, res))
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
		if (!o->operator()(*roi_, *roi_))
		{
			// FIXME: should give info on what failed
			return false;
		}
	}
	return true;
}

bool Processor::receiveRawImage(const RawImage& raw)
{
	const auto& ref {raw.cRef()};
	id_ = ref.id;

	auto info {getConversionInfo(static_cast<PxType>(ref.pixelType))};
	if (!info)
	{
		std::cerr << "could not get conversion info (ID: " << id_ << "): "
				  << "unknown pixel type: " << ref.pixelType << std::endl;
		return false;
	}
	cv::Mat tmp
	{
		ref.rows,
		ref.cols,
		info->inputType,
		ref.buffer,
		ref.step > 0ul ? ref.step : static_cast<std::size_t>(cv::Mat::AUTO_STEP)
	};

	// convert the color scheme if necessary
	if (info->colorConvCode == -1)
	{
		tmp.copyTo(*img_);
		*roi_ = *img_;	// XXX: not sure
	}
	else
	{
		cv::cvtColor(tmp, *img_, info->colorConvCode, info->outChannels);
		*roi_ = *img_;	// XXX: not sure
	}
	return true;
}

bool Processor::readImage(const std::string& path, int flags)
{
	*img_ = cv::imread(path, flags);
	*roi_ = *img_;
	//return img_->data != NULL;
	return img_->data != nullptr;	// XXX: this should be ok
}

void Processor::resetROI() const
{
	*roi_ = *img_;
}

void Processor::setROI(const Rectangle& roi) const
{
	const auto& r {roi.cRef()};
	*roi_ = img_->operator()
	(
		cv::Rect
		{
			r.left,
			r.top,
			r.right - r.left,
			r.bottom - r.top
		}
	);
}

void Processor::showImage(const std::string& title) const
{
	cv::imshow(title, *roi_);
	cv::waitKey();
}

bool Processor::writeImage(const std::string& filename) const
{
	std::vector<int> flags
	{
		cv::IMWRITE_PNG_COMPRESSION, 0,				// lowest compression level
		cv::IMWRITE_JPEG2000_COMPRESSION_X1000, 0	// lowest compression level
	};
	return cv::imwrite(filename, *roi_, flags);
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

std::unique_ptr<cv::Mat> rawToMatPtr(const RawImage& raw)
{
	const auto& ref {raw.cRef()};
	auto info {getConversionInfo(static_cast<PxType>(ref.pixelType))};
	if (!info)
	{
		std::cerr << "could not get conversion info (ID: " << ref.id << "): "
				  << "unknown pixel type: " << ref.pixelType << std::endl;
		return nullptr;
	}
	return std::unique_ptr<cv::Mat>
	{
		new cv::Mat
		{
			ref.rows,
			ref.cols,
			info->inputType,
			ref.buffer,
			ref.step > 0ul ? ref.step : static_cast<std::size_t>(cv::Mat::AUTO_STEP)
		}
	};
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
