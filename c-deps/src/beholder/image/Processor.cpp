// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/image/Processor.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <utility>
#include <vector>

#include "beholder/capi/Image.h"
#include "beholder/capi/Result.h"
#include "beholder/image/ConversionInfo.h"
#include "beholder/util/Constants.h"
#include "beholder/util/Enums.h"

namespace beholder {

namespace {
// Static checks which enforce compliance between our read modes and
// the OpenCV supported ones.
using Mod = ReadMode;
static_assert(Mod::NoChange == cv::IMREAD_UNCHANGED);
static_assert(Mod::Grayscale == cv::IMREAD_GRAYSCALE);
static_assert(Mod::Color == cv::IMREAD_COLOR);
static_assert(Mod::AnyColor == cv::IMREAD_ANYCOLOR);
static_assert(Mod::NoOrient == cv::IMREAD_IGNORE_ORIENTATION);
}  // namespace

Processor::Processor() : img_{new cv::Mat{}}, roi_{new cv::Mat{}} {
	*roi_ = *img_;
}

// NOLINTNEXTLINE(*-use-equals-default): incomplete type; must be defined here
Processor::~Processor(){};

bool Processor::decodeImage(void* buffer, std::size_t bufSize, ReadMode mode) {
	if (bufSize > std::numeric_limits<int>::max()) {
		std::cerr << "could not decode image: size too large" << std::endl;
		return false;
	}
	const int size{static_cast<int>(bufSize)};

	*img_ = cv::Mat{1, size, CV_8UC1, buffer};
	cv::imdecode(*img_, enums::to(mode), img_.get());  // yolo
	*roi_ = *img_;
	return roi_->data != nullptr;  // XXX: this should be ok
}

const std::vector<unsigned char>&
Processor::encodeImage(const std::string& ext) {
	encoding_.clear();
	cv::imencode(ext, *roi_, encoding_);
	return encoding_;
}

const cv::Mat& Processor::getImage() const { return *roi_; }

std::size_t Processor::getImageID() const { return id_; }

Image Processor::getRawImage() const {
	// WARNING: we assume that we can only have 8-bit Mono or BGR images
	return Image{id_,
				 roi_->rows,
				 roi_->cols,
				 roi_->elemSize() == 1UL ? enums::to(PxType::Mono8)
										 : enums::to(PxType::BGR8packed),
				 static_cast<void*>(roi_->data),
				 roi_->step1(),
				 roi_->elemSize() * cst::bits};
}

bool Processor::postprocess(const std::vector<Result>& res) {
	for (const auto& o : postprocessing) {
		// FIXME: should ask weather to overwrite or use a new output image
		if (!o->operator()(*roi_, *roi_, res)) {
			// FIXME: should give info on what failed
			return false;
		}
	}
	return true;
}

bool Processor::preprocess() {
	for (const auto& o : preprocessing) {
		// FIXME: should ask weather to overwrite or use a new output image
		if (!o->operator()(*roi_, *roi_)) {
			// FIXME: should give info on what failed
			return false;
		}
	}
	return true;
}

bool Processor::receiveRawImage(const Image& raw) {
	const auto& ref{raw.cRef()};
	id_ = ref.id;

	auto info{getConversionInfo(enums::from<PxType>(ref.pixelType))};
	if (!info) {
		std::cerr << "could not get conversion info (ID: " << id_ << "): "
				  << "unknown pixel type: " << ref.pixelType << std::endl;
		return false;
	}
	const cv::Mat tmp{
		ref.rows, ref.cols, info->inputType, ref.buffer,
		ref.step > 0UL ? ref.step : enums::to(cv::Mat::AUTO_STEP)};

	// convert the color scheme if necessary
	if (info->colorConvCode == -1) {
		tmp.copyTo(*img_);
		*roi_ = *img_;	// XXX: not sure
	} else {
		cv::cvtColor(tmp, *img_, info->colorConvCode, info->outChannels);
		*roi_ = *img_;	// XXX: not sure
	}
	return true;
}

bool Processor::readImage(const std::string& path, ReadMode mode) {
	*img_ = cv::imread(path, enums::to(mode));
	*roi_ = *img_;
	return img_->data != nullptr;  // XXX: this should be ok
}

void Processor::resetROI() const { *roi_ = *img_; }

void Processor::setROI(const Rectangle& roi) const {
	*roi_ = *img_;	// reset ROI

	const auto& r{roi.cRef()};
	cv::Rect crop{r.left, r.top, r.right - r.left, r.bottom - r.top};
	// snap to bounds
	crop.x = crop.x > 0 ? crop.x : 0;
	crop.x = crop.x < img_->cols ? crop.x : img_->cols - 1;
	crop.width =
		crop.x + crop.width <= img_->cols ? crop.width : img_->cols - crop.x;

	crop.y = crop.y > 0 ? crop.y : 0;
	crop.y = crop.y < img_->rows ? crop.y : img_->rows - 1;
	crop.height =
		crop.y + crop.height <= img_->rows ? crop.height : img_->rows - crop.y;

	*roi_ = img_->operator()(crop);
}

void Processor::setRotatedROI(const Rectangle& roi, double angle) const {
	*roi_ = *img_;	// reset ROI

	const auto& r{roi.cRef()};
	const cv::Point2f ctr{0.5F * static_cast<float>(r.left + r.right),
						  0.5F * static_cast<float>(r.top + r.bottom)};
	// adjust transformation matrix by adding a translation from the
	// center of rotation to the (new) image center,
	// i.e. center the text box on the image
	cv::Mat rot{cv::getRotationMatrix2D(ctr, angle, 1.0)};
	const cv::Point2f center{
		0.5F * static_cast<float>(img_->size().width - 1),
		0.5F * static_cast<float>(img_->size().height - 1)};
	const cv::Point2f shift{center - ctr};
	rot.at<double>(0, 2) += static_cast<double>(shift.x);
	rot.at<double>(1, 2) += static_cast<double>(shift.y);

	cv::Mat tmp{};
	cv::warpAffine(*img_, tmp, rot, img_->size(), cv::INTER_LINEAR,
				   cv::BORDER_REPLICATE);

	cv::Rect crop{
		cv::RotatedRect{center, cv::Size{r.right - r.left, r.bottom - r.top}, 0}
			.boundingRect()};
	// snap to bounds
	crop.x = crop.x > 0 ? crop.x : 0;
	crop.x = crop.x < tmp.cols ? crop.x : tmp.cols - 1;
	crop.width =
		crop.x + crop.width <= tmp.cols ? crop.width : tmp.cols - crop.x;

	crop.y = crop.y > 0 ? crop.y : 0;
	crop.y = crop.y < tmp.rows ? crop.y : tmp.rows - 1;
	crop.height =
		crop.y + crop.height <= tmp.rows ? crop.height : tmp.rows - crop.y;

	*roi_ = tmp(crop);
}

void Processor::showImage(const std::string& title) const {
	cv::imshow(title, *roi_);
	cv::waitKey();
}

void Processor::toColor() const {
	if (img_->channels() < 3) {
		cvtColor(*img_, *img_, cv::COLOR_GRAY2BGR, 3);
	}
	resetROI();
}

void Processor::toGrayscale() const {
	if (img_->channels() > 1) {
		cvtColor(*img_, *img_, cv::COLOR_BGR2GRAY, 1);
	}
	resetROI();
}

bool Processor::writeImage(const std::string& fname) const {
	// FIXME: flags should be adjustable
	const std::vector<int> flags{
		cv::IMWRITE_PNG_COMPRESSION, 0,			   // lowest compression level
		cv::IMWRITE_JPEG2000_COMPRESSION_X1000, 0  // lowest compression level
	};
	return cv::imwrite(fname, *roi_, flags);
}

std::unique_ptr<cv::Mat> rawToMatPtr(const Image& raw) {
	const auto& ref{raw.cRef()};
	auto info{getConversionInfo(enums::from<PxType>(ref.pixelType))};
	if (!info) {
		std::cerr << "could not get conversion info (ID: " << ref.id << "): "
				  << "unknown pixel type: " << ref.pixelType << std::endl;
		return nullptr;
	}
	return std::make_unique<cv::Mat>(
		ref.rows, ref.cols, info->inputType, ref.buffer,
		ref.step > 0UL ? ref.step : enums::to(cv::Mat::AUTO_STEP));
}

}  // namespace beholder
