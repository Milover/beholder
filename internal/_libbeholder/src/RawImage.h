/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A simple, neutral class for transferring raw image data from a Camera.

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_RAW_IMAGE_H
#define BEHOLDER_RAW_IMAGE_H

#include <cstddef>
#include <cstdint>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                         Class RawImage Declaration
\*---------------------------------------------------------------------------*/

class RawImage
{
public:

	// Public data

		std::size_t id;
		int rows;
		int cols;
		std::int64_t pixelType;
		void* buffer;
		std::size_t step;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
