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

		//- Recognized/detected text
		std::string text;

		//- Bounding boxes detected by algorithms and/or NNs
		Rectangle box;

		//- Confidence of the result
		double confidence;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
