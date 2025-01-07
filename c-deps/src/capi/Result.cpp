/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cstddef>

#include "Result.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{
namespace capi
{
extern "C"
{

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

void Result_Delete(Result* r)
{
	if (r->text)
	{
		delete[] r->text;
		r->text = nullptr;
	}
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace capi
} // End extern "C"
} // End namespace beholder

// ************************************************************************* //
