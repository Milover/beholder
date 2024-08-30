/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A stupid rectangle class.

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_RECTANGLE_H
#define BEHOLDER_RECTANGLE_H

#ifdef __cplusplus
#include "CWrapperBase.h"
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
                      Struct Rectangle Declaration
\*---------------------------------------------------------------------------*/

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
}
Rectangle;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef __cplusplus
} // End extern "C"
} // End namespace capi

/*---------------------------------------------------------------------------*\
                      Class Rectangle Declaration
\*---------------------------------------------------------------------------*/

namespace capi
{
	namespace detail
	{
		struct RectangleCtor
		{
			capi::Rectangle operator()()
			{
				return capi::Rectangle {0, 0, 0, 0};
			}
		};
	}
}

//- The actual class we use throughout the library.
using Rectangle = capi::internal::CWrapperBase
<
	capi::Rectangle, capi::detail::RectangleCtor
>;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
