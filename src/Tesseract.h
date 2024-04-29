/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A wrapper for TessBaseAPI.

SourceFiles
	TessBaseAPI.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_TESSERACT_H
#define OCR_TESSERACT_H

#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <tesseract/baseapi.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

/*---------------------------------------------------------------------------*\
                      Class Tesseract Declaration
\*---------------------------------------------------------------------------*/

class Tesseract
{
private:
	using TBA = tesseract::TessBaseAPI;

	// Private data

		//- A pointer to the tesseract api
		TBA* p_;


public:

	// Constructors

		//- Default constructor
		Tesseract();

		//- Disable copy constructor
		Tesseract(const Tesseract&) = delete;

		//- Disable move constructor
		Tesseract(Tesseract&&) = delete;


	//- Destructor
	~Tesseract();

	// Member functions

		//- Reset Tesseract
		void clear();

		//- Detect text
		std::vector<cv::Rect> detectText();

		//- Initialize Tesseract
		bool init(const Config& cfg);

		//- Run OCR and get result as UTF8 char*.
		//	The reciever takes ownership and must call delete[].
		char* recognizeText();

		//- Run OCR and get result as UTF8 string
		std::string recognizeTextStr();

		//- Set image for recognition
		void setImage(const cv::Mat& im, int bytesPerPixel = 1);


	// Member operators

		//- Disable copy assignment
		Tesseract& operator=(const Tesseract&) = delete;

		//- Disable move assignment
		Tesseract& operator=(Tesseract&&) = delete;

};

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
