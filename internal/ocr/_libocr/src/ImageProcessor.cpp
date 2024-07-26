/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "ImageProcessor.h"
#include "OcrResults.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

ImageProcessor::ImageProcessor()
:
	img_ {new cv::Mat{}}
{}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

ImageProcessor::~ImageProcessor()
{
	delete img_;
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void ImageProcessor::copyBayerRGGB8(int rows, int cols, void* buf, size_t step)
{
	if (step < 1) {
		step = cv::Mat::AUTO_STEP;
	}
	cv::Mat tmp {rows, cols, CV_8UC1, buf, step};
	cv::cvtColor(tmp, *img_, cv::COLOR_BayerRGGB2BGR, 3);
}

bool ImageProcessor::decodeImage(void* buffer, int bufSize, int flags)
{
	*img_ = cv::Mat{1, bufSize, CV_8UC1, buffer};
	cv::imdecode(*img_, flags, img_);	// yolo
	return img_->data != NULL;
}


const cv::Mat& ImageProcessor::getImage() const
{
	return *img_;
}


bool ImageProcessor::postprocess(const OcrResults& res)
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


bool ImageProcessor::preprocess()
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

void ImageProcessor::receiveMono8(int rows, int cols, void* buf, size_t step)
{
	if (step < 1) {
		step = cv::Mat::AUTO_STEP;
	}
	*img_ = cv::Mat {rows, cols, CV_8UC1, buf, step};
}

bool ImageProcessor::readImage(const std::string& path, int flags)
{
	*img_ = cv::imread(path, flags);
	return img_->data != NULL;
}

void ImageProcessor::showImage(const std::string& title) const
{
	cv::imshow(title, *img_);
	cv::waitKey();
}


bool ImageProcessor::writeImage(const std::string& filename) const
{
	return cv::imwrite(filename, *img_);
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
