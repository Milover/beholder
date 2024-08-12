/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A wrapper class for image processing functions.

SourceFiles
	Processor.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_PROCESSOR_H
#define BEHOLDER_PROCESSOR_H

#include <array>
#include <cstddef>
#include <utility>
#include <string>
#include <vector>

#include "ConversionInfo.h"
#include "ProcessingOp.h"
#include "RawImage.h"
#include "Result.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                         Class Processor Declaration
\*---------------------------------------------------------------------------*/

class Processor
{
private:

	// Private data

		//- An image
		//	NOTE: a unique_ptr would be nicer, but cgo keeps complaining
		cv::Mat* img_;

		//- The camera assigned ID of the current image.
		//	FIXME: only images received from a camera will have an ID.
		std::size_t id_ {0};

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
		Processor();

		//- Disable copy constructor
		Processor(const Processor&) = delete;

		//- Disable move constructor
		Processor(Processor&&) = delete;

	//- Destructor
	~Processor();

	// Member functions

		//- Decode an image from a buffer.
		//	The buffer is left unchanged and the data is copied into the
		//	Processor. New memory is not allocated if the Processor
		//	has enough space to hold the decoded image.
		bool decodeImage(void* buffer, int bufSize, int flags);

		//- Get the stored image
		const cv::Mat& getImage() const;

		//- Get the stored image ID.
		//	NOTE: the image ID is assigned by a camera device, hence
		//	only images received as acquisition results, i.e. as a result
		//	of calling receiveAcquisitionResult(...), will have an ID.
		std::size_t getImageID() const;

		//- Run pre-OCR image processing
		//	FIXME: this should take an Image
		bool preprocess();

		//- Run post-OCR image processing
		//	FIXME: this should take an Image
		bool postprocess(const std::vector<Result>& res);

		//- Recieve a raw image, usually a camera acquisition result, and
		//	copy it locally while converting to a standard color space.
		//
		//	Returns false if image conversion fails, and true otherwise.
		bool receiveRawImage(const RawImage& img);

		//- Read an image from disc
		bool readImage(const std::string& path, int flags);

		//- Show an image and wait for a keypress
		void showImage(const std::string& title = "image") const;

		//- Write an image to disc
		//	FIXME: hard-coded to use the lowest compression levels for PNG/JPEG.
		bool writeImage(const std::string& filename = "img.png") const;


	// Member operators

		//- Disable copy assignment
		Processor& operator=(const Processor&) = delete;

		//- Disable move assignment
		Processor& operator=(Processor&&) = delete;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
