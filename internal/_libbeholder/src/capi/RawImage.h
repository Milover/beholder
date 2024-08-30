/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A stupid image class.

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_RAW_IMAGE_H
#define BEHOLDER_RAW_IMAGE_H

#ifdef __cplusplus
#include <cstdint>

#include "CWrapperBase.h"
#else
#include <stdint.h>
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef __cplusplus
namespace beholder
{
namespace capi
{
extern "C"
{
#endif

/*---------------------------------------------------------------------------*\
                         Struct RawImage Declaration
\*---------------------------------------------------------------------------*/

typedef struct
{
	size_t id;
	int rows;
	int cols;
	// FIXME: this is pretty horrible, however simple values make
	// life easier from the Go side
	int64_t pixelType;
	// FIXME: this should be something better than a raw pointer,
	// however we can't do weak/shared pointers since Go will probably
	// manage this memory, so should think of something, because it
	// will cause issues
	// TODO: should have the buffer size
	void* buffer;
	size_t step;
	//- Number of bits to store a pixel.
	//	Cameras can return packed pixel types, so bytes are inappropriate.
	size_t bitsPerPixel;
}
RawImage;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef __cplusplus
} // End extern "C"
} // End namespace capi

/*---------------------------------------------------------------------------*\
                          Class RawImage Declaration
\*---------------------------------------------------------------------------*/

namespace capi
{
	namespace detail
	{
		struct RawImageCtor
		{
			capi::RawImage operator()()
			{
				return capi::RawImage {0ul, 0, 0, 0, nullptr, 0ul, 0ul};
			}
		};
	}
}

//- The actual class we use throughout the library.
using RawImage = capi::internal::CWrapperBase
<
	capi::RawImage, capi::detail::RawImageCtor
>;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
