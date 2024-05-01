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

#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>

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
		cv::Mat img_;

	// Private Member functions

public:

	// Constructors

	//- Destructor

	// Member functions

		//- Draw rectangles onto an image
		void drawRectangles
		(
			const std::vector<cv::Rect>& rects,
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
		bool readImage
		(
			const std::string& path,
			int flags = cv::IMREAD_GRAYSCALE
		);

		//- Show an image and wait for a keypress
		void showImage(const std::string& title = "image") const;

	// Member operators

};

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
