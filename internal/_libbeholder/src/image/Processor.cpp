/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <pylon/GrabResultPtr.h>
#include <pylon/ImagePersistence.h>

#include "ConversionInfo.h"
#include "Processor.h"
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

bool Processor::postprocess(const Result& res)
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

bool Processor::receiveAcquisitionResult(const Pylon::CGrabResultPtr& r)
{
	id_ = r->GetID();

	// find the conversion table entry
	Pylon::EPixelType pixelType {r->GetPixelType()};
	auto found
	{
		std::find_if
		(
			ConversionInfoTable.begin(),
			ConversionInfoTable.end(),
			[pixelType](const auto& p) -> bool { return p.first == pixelType; }
		)
	};
	if (found == ConversionInfoTable.end())
	{
		std::cerr << "could not receive acquisition result (ID: " << id_ << "): "
				  << "unknown pixel type: " << pixelType << std::endl;
		return false;
	}
	const ConversionInfo& info {found->second};

	// assign the buffer
	std::size_t step;
	cv::Mat tmp
	{
		static_cast<int>(r->GetHeight()),
		static_cast<int>(r->GetWidth()),
		info.inputType,
		r->GetBuffer(),
		r->GetStride(step) ? step : static_cast<std::size_t>(cv::Mat::AUTO_STEP)
	};

	// convert the color scheme if necessary
	if (info.colorConvCode == -1)
	{
		tmp.copyTo(*img_);
	}
	else
	{
		cv::cvtColor(tmp, *img_, info.colorConvCode, info.outChannels);
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

bool Processor::writeAcquisitionResult
(
	const Pylon::CGrabResultPtr& r,
	const std::string& filename
) const
{
	try
	{
		Pylon::CImagePersistence::Save
		(
			Pylon::ImageFileFormat_Png, filename.c_str(), r
		);
		return true;
	}
	catch(...) { }
	return false;
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

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
