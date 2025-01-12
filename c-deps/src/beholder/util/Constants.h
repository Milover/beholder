// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Common constants.

#ifndef BEHOLDER_UTIL_CONSTANTS_H
#define BEHOLDER_UTIL_CONSTANTS_H

#include <limits>
#include <type_traits>

namespace beholder {
namespace cst {

namespace detail {
template<typename T>
using enable_if_arith = std::enable_if_t<std::is_arithmetic_v<T>, T>;
}

// The number of bits in a byte.
template<typename T>
inline constexpr detail::enable_if_arith<T> bits_v{8};

inline constexpr std::size_t bits = bits_v<std::size_t>;

// Max 8-bit number.
inline constexpr auto max8bit = std::numeric_limits<unsigned char>::max();

}  // namespace cst
}  // namespace beholder

#endif	// BEHOLDER_UTIL_CONSTANTS_H
