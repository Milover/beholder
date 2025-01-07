/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	Handy-dandy type traits.

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_TRAITS_H
#define BEHOLDER_TRAITS_H

#include <type_traits>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * * * * Type Traits * * * * * * * * * * * * * * * * //

template<typename T>
using removeCVRef_t = std::remove_reference_t<std::remove_cv_t<T>>;

template<typename Tcvref, typename T>
using enableIfSame_t = std::enable_if_t<std::is_same_v<removeCVRef_t<Tcvref>, T>>;

//- Cast an enum value to it's underlying type
template<typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
inline constexpr typename std::underlying_type_t<E>
toUnderlying(E e)
{
   return static_cast<typename std::underlying_type_t<E>>(e);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
