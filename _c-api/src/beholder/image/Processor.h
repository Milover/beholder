// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Main class for handling (traditional) image processing.

#ifndef BEHOLDER_IMAGE_PROCESSOR_H
#define BEHOLDER_IMAGE_PROCESSOR_H

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "beholder/capi/Image.h"
#include "beholder/capi/Rectangle.h"
#include "beholder/capi/Result.h"
#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

// Image read modes.
// Can be combined using bitwise OR (|), although all combinations are not
// necessarily valid.
enum class ReadMode {
	NoChange = -0x01,  // keep alpha channel and ignore EXIF orientation
	Grayscale = 0x00,  // convert image to single-channel grayscale
	Color = 0x01,	   // convert image to BGR
	AnyColor = 0x04,   // use any possible color format
	NoOrient = 0x80	   // ignore EXIF orientation
};

// TODO: Processor 'owns' the image entirely at this point, it should instead
// just handle image processing and release the image to us afterwards.
// TODO: We should also remove and/or hide the distinction between
// a 'raw' image and a cv::Mat, which we treat as the 'true' image currently.
class Processor {
private:
	std::unique_ptr<cv::Mat> img_;		   // underlying image
	std::unique_ptr<cv::Mat> roi_;		   // active image ROI
	std::vector<unsigned char> encoding_;  // local encoding buffer
	// FIXME: only images received from a camera will have an ID.
	// It's probably better that we handle ID tagging entirely.
	std::size_t id_{0};	 // camera assigned ID of the current image.

public:
	using OpList = std::vector<ProcessingOp::OpPtr>;

	// TODO: we probably don't want to distinguish between
	// pre- and post-processing. We should just execute the list of operations
	// as they're provided and let the user define the pipeline.
	OpList preprocessing;	// list of preprocessing operations
	OpList postprocessing;	// list of postprocessing operations

	// Default constructor
	Processor();

	Processor(const Processor&) = delete;
	Processor(Processor&&) = delete;

	// Default destructor.
	// Defined in the source because unique_ptr complains about
	// incomplete types.
	~Processor();

	Processor& operator=(const Processor&) = delete;
	Processor& operator=(Processor&&) = delete;

	// Decode an image from a buffer.
	// The buffer is left unchanged and the data is copied into the
	// Processor. New memory is not allocated if the Processor
	// has enough space to hold the decoded image.
	bool decodeImage(void* buffer, std::size_t bufSize, ReadMode mode);

	// Encode an image (current ROI) into a local buffer in the specified
	// format, and return the encoding.
	const std::vector<unsigned char>&
	encodeImage(const std::string& ext = ".png");

	// Get the stored image
	[[nodiscard]] const cv::Mat& getImage() const;

	// Get the stored image ID.
	// NOTE: the image ID is assigned by a camera device, hence
	// only images received as acquisition results, i.e. as a result
	// of calling receiveAcquisitionResult(...), will have an ID.
	[[nodiscard]] std::size_t getImageID() const;

	// Get the stored image as an Image
	[[nodiscard]] Image getRawImage() const;

	// Run pre-OCR image processing
	// FIXME: this should take an Image
	// FIXME: should be merged with postprocess
	bool preprocess();

	// Run post-OCR image processing
	// FIXME: this should take an Image
	// FIXME: should be merged with preprocess
	bool postprocess(const std::vector<Result>& res);

	// Recieve a raw image, usually a camera acquisition result, and
	// copy it locally while converting to a standard color space.
	//
	// Returns false if image conversion fails, and true otherwise.
	bool receiveRawImage(const Image& raw);

	// Read an image from disc
	bool readImage(const std::string& path, ReadMode mode);

	// Reset the region of interest, i.e. set the ROI to the whole image
	void resetROI() const;

	// Set the region of interest
	// FIXME: bad implementation, always operates on the original image,
	// not the current ROI
	void setROI(const Rectangle& roi) const;

	// Set the region of interest
	// FIXME: bad implementation, always operates on the original image,
	// not the current ROI
	void setRotatedROI(const Rectangle& roi, double angle) const;

	// Convert image to color (BGR) and reset the ROI.
	void toColor() const;

	// Convert image to grayscale and reset the ROI
	void toGrayscale() const;

	// Write an image (current ROI) to disc
	// FIXME: hard-coded to use the lowest compression levels for PNG/JPEG.
	[[nodiscard]] bool writeImage(const std::string& fname = "img.png") const;
};

// Convert a raw image to a cv::Mat pointer.
std::unique_ptr<cv::Mat> rawToMatPtr(const Image& raw);

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_PROCESSOR_H
