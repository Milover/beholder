// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Common utility functions.

#ifndef BEHOLDER_UTIL_UTILITY_H
#define BEHOLDER_UTIL_UTILITY_H

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace beholder {

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

}  // namespace beholder

#endif	// BEHOLDER_UTIL_UTILITY_H
