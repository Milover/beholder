// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Stuff shared between tests

#ifndef BEHOLDER_TEST_TESTING_H
#define BEHOLDER_TEST_TESTING_H

// Report sanitizer errors.
extern "C" {
void __ubsan_on_report();  // NOLINT
void __asan_on_report();   // NOLINT
void __msan_on_report();   // NOLINT
void __tsan_on_report();   // NOLINT
}  // extern "C"

#endif	// BEHOLDER_TEST_TESTING_H
