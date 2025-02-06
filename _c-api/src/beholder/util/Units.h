// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Common units and conversions.

#ifndef BEHOLDER_UTIL_UNITS_H
#define BEHOLDER_UTIL_UNITS_H

#include <cstddef>

namespace beholder {

constexpr size_t KiB = 1024;
constexpr size_t MiB = 1024 * KiB;
constexpr size_t GiB = 1024 * MiB;

constexpr size_t operator""_KiB(unsigned long long val) { return val * KiB; }
constexpr size_t operator""_MiB(unsigned long long val) { return val * MiB; }
constexpr size_t operator""_GiB(unsigned long long val) { return val * GiB; }

}  // namespace beholder

#endif	// BEHOLDER_UTIL_UNITS_H
