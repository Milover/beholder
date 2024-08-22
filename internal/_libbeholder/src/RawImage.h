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

		std::size_t id {0ul};
		int rows {0};
		int cols {0};
		// FIXME: this is pretty horrible, however simple values make
		// life easier from the Go side
		std::int64_t pixelType {0};
		// FIXME: this should be something better than a raw pointer,
		// however we can't do weak/shared pointers since Go will probably
		// manage this memory, so should think of something, because it
		// will cause issues
		void* buffer {nullptr};
		std::size_t step {0ul};
		// TODO: should have the size of the buffer in bytes

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
