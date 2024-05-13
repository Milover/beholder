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

#include "ImageProcessor.h"
#include "OcrResults.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace tesseract
{
	class TessBaseAPI;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

/*---------------------------------------------------------------------------*\
                      Class Tesseract Declaration
\*---------------------------------------------------------------------------*/

class Tesseract
{
private:

	// Private data

		//- A pointer to the tesseract api
		//	NOTE: a unique_ptr would be nicer, but cgo keeps complaining
		tesseract::TessBaseAPI* p_;

		//- OCR results
		OcrResults res_;

public:

	// Public data

		//- Configuration file paths
		std::vector<std::string> configPaths;
		//- Directory path of the model (trained data) file
		std::string modelPath;
		//- Model (trained data) name
		std::string model;

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

		//- Reset Tesseract and OCR results
		void clear();

		//- Run text detection and store the results
		bool detectText();

		//- Get a const reference to the OCR results
		const OcrResults& getResults() const;

		//- Initialize Tesseract
		bool init();

		//- Run text recognition and store the results
		bool recognizeText();

		//- Run text detection and recognition and store the results
		bool detectAndRecognize();

		//- Set image for recognition
		void setImage(const ImageProcessor& ip, int bytesPerPixel = 1);


	// Member operators

		//- Disable copy assignment
		Tesseract& operator=(const Tesseract&) = delete;

		//- Disable move assignment
		Tesseract& operator=(Tesseract&&) = delete;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
