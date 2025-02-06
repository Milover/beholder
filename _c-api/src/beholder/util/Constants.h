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

template<typename T>
using enable_if_floating = std::enable_if_t<std::is_floating_point_v<T>, T>;

}  // namespace detail

// The number of bits in a byte.
template<typename T>
constexpr detail::enable_if_arith<T> bits_v{8};

constexpr size_t bits = bits_v<size_t>;

// Degrees constants.
template<typename T>
constexpr detail::enable_if_floating<T> deg90_v{90};
template<typename T>
constexpr detail::enable_if_floating<T> deg180_v{180};
template<typename T>
constexpr detail::enable_if_floating<T> deg270_v{270};
template<typename T>
constexpr detail::enable_if_floating<T> deg360_v{360};

constexpr double deg90 = deg90_v<double>;
constexpr double deg180 = deg180_v<double>;
constexpr double deg270 = deg270_v<double>;
constexpr double deg360 = deg360_v<double>;

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
