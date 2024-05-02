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

#include "Rectangle.h"

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

		//- Draw rectangles onto an image
		void drawRectangles
		(
			const std::vector<Rectangle>& rects,
			const Config& cfg
		);

		//- Get the stored image
		const cv::Mat& getImage() const;

		//- Normalize brightness and contrast
		//	TODO: this needs to be refactored
		void normalize(float clipPct = 0.5);

		//- Prepare an image for OCR
		//	TODO: this needs to be able to adjust execution based on a Config
		bool preprocess(const Config& cfg);

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

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
