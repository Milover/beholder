// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Handy-dandy type traits.

#ifndef BEHOLDER_UTIL_TRAITS_H
#define BEHOLDER_UTIL_TRAITS_H

#include <type_traits>

namespace beholder {

template<typename T>
using remove_cvref_t = std::remove_reference_t<std::remove_cv_t<T>>;

template<typename T, typename U>
using enable_if_same_t = std::enable_if_t<std::is_same_v<remove_cvref_t<T>, U>>;

}  // namespace beholder

#endif	// BEHOLDER_UTIL_TRAITS_H
