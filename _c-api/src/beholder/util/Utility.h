// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Common utility functions.

#ifndef BEHOLDER_UTIL_UTILITY_H
#define BEHOLDER_UTIL_UTILITY_H

#include <beholder/util/Constants.h>

#include <charconv>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>

namespace beholder {

using CCharSpan = std::span<const char>;

// Helper function for setting ch to raw.
// If ch != nullptr, calls delete[] on ch.
void chPtrFromLiteral(char*& ch, const char* lit);

// Convert a vector of strings into an array of char*.
// NOLINTNEXTLINE(*-c-arrays)
std::unique_ptr<char*[]> vecStr2ChPtrArr(const std::vector<std::string>& v);

// Trim leading whitespace (left trim).
void trimWhiteL(std::string& s);

// Trim trailing whitespace (right trim).
void trimWhiteR(std::string& s);

// Trim leading and trailing whitespace (left-right trim).
void trimWhiteLR(std::string& s);

// TODO: the return type should wrap the value and a possible error.
template<
	typename T, int Base = cst::charconv::base10,
	std::enable_if_t<std::is_arithmetic_v<T> && Base >= cst::charconv::base2 &&
						 Base <= cst::charconv::base32,
					 bool> = true>
T toDecimal(CCharSpan chars) {
	T val{};
	const char* begin{&(*chars.begin())};
	const char* end{&(*chars.end())};
	std::from_chars_result res{std::from_chars(begin, end, val, Base)};

	if (res.ec != std::errc{}) {
		const std::error_code e{std::make_error_code(res.ec)};
		std::cerr << "error (" << e.value() << "): " << e.message()
				  << std::endl;	 // NOLINT(performance-avoid-endl)
	}
	return val;
}

}  // namespace beholder

#endif	// BEHOLDER_UTIL_UTILITY_H
