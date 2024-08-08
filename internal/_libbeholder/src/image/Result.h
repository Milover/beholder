/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A simple class for holding various image detection results

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_RESULT_H
#define BEHOLDER_RESULT_H

#include <string>
#include <vector>

#include "Rectangle.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                          Class Result Declaration
\*---------------------------------------------------------------------------*/

class Result
{
public:

	// Public data

		// TODO: should we hold an image or a reference to an image?
		// TODO: should have confidences also
		// TODO: should have simple values and then use as std::vector<Result>

		//- Recognized/detected text
		std::vector<std::string> tags;

		//- Bounding boxes detected by algorithms and/or NNs
		std::vector<Rectangle> boundingBoxes;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
