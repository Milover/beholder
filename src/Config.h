/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A configuration data class.

SourceFiles
	Config.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_CONFIG_H
#define OCR_CONFIG_H

#include <array>
#include <string>
#include <utility>
#include <vector>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

/*---------------------------------------------------------------------------*\
                          Class Config Declaration
\*---------------------------------------------------------------------------*/

class Config
{
private:

	// Private data

public:

	// Public data

		//- Paths of Tesseract configuration files
		std::vector<std::string> configPaths;
		//- Path to Tesseract model file directory
		std::string modelPath;
		//- Name of the Tesseract model
		std::string model;
		//- The RGBA color of the text box drawn during text detection
		std::array<double, 4> textBoxColor;
		//- The line thickness of the text box drawn during text detection
		int textBoxThickness;

	// Constructors

	//- Destructor

	// Member functions

		//- Parse data from a JSON char*
		bool parse(const char* s);

		//- Parse data from a JSON string
		bool parse(const std::string& s);

	// Member operators

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
