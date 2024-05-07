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

		//- Get the stored image
		const cv::Mat& getImage() const;

		//- Run pre-OCR image processing
		bool preprocess();

		//- Run post-OCR image processing
		bool postprocess(const OcrResults& res);

		//- Read an image from disc
		bool readImage(const std::string& path, int flags);

		//- Show an image and wait for a keypress
		void showImage(const std::string& title = "image") const;


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
