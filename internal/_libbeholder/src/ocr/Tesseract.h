/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A wrapper for TessBaseAPI.

SourceFiles
	TessBaseAPI.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_TESSERACT_H
#define BEHOLDER_TESSERACT_H

#include <string>
#include <utility>
#include <vector>

#include "Processor.h"
#include "Result.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace tesseract
{
	class TessBaseAPI;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
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
		//	FIXME: should be a std::vector<Result>
		std::vector<Result> res_;

public:

	// Public data

		//- Configuration file paths
		std::vector<std::string> configPaths;
		//- Directory path of the model (trained data) file
		std::string modelPath;
		//- Model (trained data) name
		std::string model;
		//- Page segmentation mode
		int pageSegMode {6};	// PSM_SINGLE_BLOCK, which is the default
		//- A map of settable variables
		std::vector<std::pair<std::string, std::string>> variables
		{
			{"load_system_dawg", "0"},
			{"load_freq_dawg", "0"}
		};

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

		//- Dump Tesseract variables to stdout.
		void dumpVariables() const;

		//- Get No. dawgs.
		int getNoDawgs() const;

		//- Get a const reference to the OCR results
		const std::vector<Result>& getResults() const;

		//- Initialize Tesseract
		bool init();

		//- Run text detection if necessary, then run text recognition
		//	and store the results.
		//
		//	Leading and trailing white space is trimmed from
		//	the recognized text.
		bool recognizeText();

		//- Set image for detection/recognition and clear all results.
		//	FIXME: this should take an Image
		void setImage(const Processor& ip, int bytesPerPixel = 1);


	// Member operators

		//- Disable copy assignment
		Tesseract& operator=(const Tesseract&) = delete;

		//- Disable move assignment
		Tesseract& operator=(Tesseract&&) = delete;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
