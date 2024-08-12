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

bool Processor::receiveRawImage(const RawImage& img)
{
	id_ = img.id;

	// find the conversion table entry
	PxType typ {static_cast<PxType>(img.pixelType)};
	auto found
	{
		std::find_if
		(
			ConversionInfoTable.begin(),
			ConversionInfoTable.end(),
			[typ](const auto& p) -> bool { return p.first == typ; }
		)
	};
	if (found == ConversionInfoTable.end())
	{
		std::cerr << "could not receive acquisition result (ID: " << id_ << "): "
				  << "unknown pixel type: " << typ << std::endl;
		return false;
	}
	const ConversionInfo& info {found->second};

	// assign the buffer
	cv::Mat tmp
	{
		img.rows,
		img.cols,
		info.inputType,
		img.buffer,
		img.step > 0ul ? img.step : static_cast<std::size_t>(cv::Mat::AUTO_STEP)
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
