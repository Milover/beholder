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
	size_t id {0ul};
	int rows {0};
	int cols {0};
	// FIXME: this is pretty horrible, however simple values make
	// life easier from the Go side
	int64_t pixelType {0};
	// FIXME: this should be something better than a raw pointer,
	// however we can't do weak/shared pointers since Go will probably
	// manage this memory, so should think of something, because it
	// will cause issues
	void* buffer {nullptr};
	size_t step {0ul};
	// TODO: should have the size of the buffer in bytes
}
RawImage;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef __cplusplus
} // End namespace capi
} // End namespace beholder
} // End extern "C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
