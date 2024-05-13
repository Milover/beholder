/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A simple class for holding OCR results

\*---------------------------------------------------------------------------*/

#ifndef OCR_OCR_RESULTS_H
#define OCR_OCR_RESULTS_H

#include <string>
#include <vector>

#include "Rectangle.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

/*---------------------------------------------------------------------------*\
                         Class OcrResults Declaration
\*---------------------------------------------------------------------------*/

class OcrResults
{
public:

	// Public data

		//- Recognized text
		std::string text;

		//- Text boxes
		std::vector<Rectangle> textBoxes;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
