/*---------------------------------------------------------------------------*\

	turbo - Copyright (C) 2019 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	General use alias templates

\*---------------------------------------------------------------------------*/

#ifndef OCR_GENERAL_H
#define OCR_GENERAL_H

#include <type_traits>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * * * * Typedefs  * * * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Type Traits * * * * * * * * * * * * * * * * //

//- Remove const, volatile and reference from a type
template<typename T>
using removeCVRef_t = std::remove_cv_t<std::remove_reference_t<T>>;


// * * * * * * * * * * * * * * * * Flags * * * * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
