// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_UTIL_ERRORS_H
#define BEHOLDER_UTIL_ERRORS_H

#include <system_error>

namespace beholder {
namespace err {

// Construct an error_code from an errno code.
std::error_code fromErrno(int errnoCode);

// Pretty-print an error category, code and message.
void printErr(const std::error_code& err);

}  // namespace err
}  // namespace beholder

#endif	// BEHOLDER_UTIL_ERRORS_H
