/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A wrapper class for image processing functions.

SourceFiles
	ImageProcessor.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_IMAGE_PROCESSOR_H
#define OCR_IMAGE_PROCESSOR_H

#include <cstddef>
#include <vector>
#include <string>
#include <vector>

#include "OcrResults.h"
#include "ProcessingOp.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

/*---------------------------------------------------------------------------*\
                    Class ImageProcessor Declaration
\*---------------------------------------------------------------------------*/

class ImageProcessor
{
private:

	// Private data

		//- An image
		//	NOTE: a unique_ptr would be nicer, but cgo keeps complaining
		cv::Mat* img_;

	// Private Member functions 

public:

	using OpList = std::vector<ProcessingOp::OpPtr>;

	//- Public data

		//- A list of preprocessing operations
		OpList preprocessing;
		//- A list of postprocessing operations
		OpList postprocessing;

	// Constructors

		//- Default constructor
		ImageProcessor();

		//- Disable copy constructor
		ImageProcessor(const ImageProcessor&) = delete;

		//- Disable move constructor
		ImageProcessor(ImageProcessor&&) = delete;

	//- Destructor
	~ImageProcessor();

	// Member functions

		//- Decode an image from a buffer.
		//	The buffer is left unchanged and the data is copied into the
		//	ImageProcessor. New memory is not allocated if the ImageProcessor
		//	has enough space to hold the decoded image.
		bool decodeImage(void* buffer, int bufSize, int flags);

		//- Get the stored image
		const cv::Mat& getImage() const;

		//- Run pre-OCR image processing
		bool preprocess();

		//- Run post-OCR image processing
		bool postprocess(const OcrResults& res);

		//- Read an image from disc
		bool readImage(const std::string& path, int flags);

		//- Copy a Bayer RGGB 8-bit image locally and convert it into a
		//	3-channel BGR image (OpenCV standard)
		void copyBayerRGGB8(int rows, int cols, void* buf, size_t step);

		//- Show an image and wait for a keypress
		void showImage(const std::string& title = "image") const;

		//- Write an image to disc
		bool writeImage(const std::string& filename = "img.png") const;


	// Member operators

		//- Disable copy assignment
		ImageProcessor& operator=(const ImageProcessor&) = delete;

		//- Disable move assignment
		ImageProcessor& operator=(ImageProcessor&&) = delete;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
