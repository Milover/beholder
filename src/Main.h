/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	Global variables for the main loop of the program

SourceFiles
	Main.cpp

\*---------------------------------------------------------------------------*/

#ifndef MAIN_H
#define MAIN_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{


// * * * * * * * * * * * * * Global Variables  * * * * * * * * * * * * * * * //

static const char* const HEADER
{
	"\nocr - Copyright (C) 2024 P. Milovic\n\n"
};


static const char* const USAGE
{
	"Usage:\n\tunimplemented\n\n"
	"Description:\n\tunimplemented\n\n"
};

// FIXME: hardcoded traineddata dir
static constexpr char const* tessdataDir
{
	"/Users/philipp/Documents/c++/projects/ocr/assets/tessdata/dotmatrix"
};

// FIXME: hardcoded config dir
static constexpr char const* patternConfig
{
	"/Users/philipp/Documents/c++/projects/ocr/assets/test.patterns.config"
};

// FIXME: hardcoded test image
static constexpr char const* testPhoto
{
	//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/simple_12pt.tif"
	//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.0.raw.jpg"
	//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.0.clean.jpg"
	//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.1.png"
	//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.2.jpg"
	//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.3.jpeg"
	//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.3.cropped.jpg"
	//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.4.png"
	//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.5.png"
	//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.6.jpeg"
	"/Users/philipp/Documents/c++/projects/ocr/test/testdata/dot_matrix.7.bmp"
	//"/Users/philipp/Documents/c++/projects/ocr/test/testdata/random.jpg"
};

// FIXME: hardcoded model name
static constexpr char const* modelName
{
	//"5x5_Dots_FT_500"
	"dotOCRDData1"
	//"Orario_FT_500"
	//"Transit_FT_500"
	//"Dotrice_FT_500"
	//"LCDDot_FT_500"
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
