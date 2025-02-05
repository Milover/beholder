// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Common constants.

#ifndef BEHOLDER_UTIL_CONSTANTS_H
#define BEHOLDER_UTIL_CONSTANTS_H

#include <cstddef>
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
constexpr detail::enable_if_arith<T> bits_v{8};

constexpr size_t bits = bits_v<size_t>;

// Max 8-bit number.
constexpr auto max8bit = std::numeric_limits<unsigned char>::max();

namespace charconv {

constexpr int defaultBase{10};
constexpr int minBase{2};
constexpr int maxBase{32};

}  // namespace charconv

}  // namespace cst
}  // namespace beholder

#endif	// BEHOLDER_UTIL_CONSTANTS_H
