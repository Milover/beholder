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
} // End namespace capi
} // End namespace beholder
} // End extern "C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
