// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Common units and conversions.

#ifndef BEHOLDER_UTIL_UNITS_H
#define BEHOLDER_UTIL_UNITS_H

#include <cstddef>
#include <numbers>

#include "beholder/util/Constants.h"

namespace beholder {

constexpr size_t operator""_bits_to_bytes(unsigned long long val) {
	return val * cst::bits_v<size_t>;
}
constexpr size_t operator""_bytes_to_bits(unsigned long long val) {
	return val / cst::bits_v<size_t>;
}

constexpr long double operator""_deg_to_rad(long double deg) {
	return deg * std::numbers::pi_v<long double> / cst::deg180_v<long double>;
}
constexpr long double operator""_rad_to_deg(long double rad) {
	return rad * cst::deg180_v<long double> / std::numbers::pi_v<long double>;
}

constexpr size_t KiB = 1024;
constexpr size_t MiB = 1024 * KiB;
constexpr size_t GiB = 1024 * MiB;

constexpr size_t operator""_KiB(unsigned long long val) { return val * KiB; }
constexpr size_t operator""_MiB(unsigned long long val) { return val * MiB; }
constexpr size_t operator""_GiB(unsigned long long val) { return val * GiB; }

}  // namespace beholder

#endif	// BEHOLDER_UTIL_UNITS_H
