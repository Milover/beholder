// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "Testing.h"

#include <gtest/gtest.h>

// Report sanitizer errors.
extern "C" {
void __ubsan_on_report() { FAIL() << "Got UBSan error"; }
void __asan_on_report() { FAIL() << "Got ASan error"; }
void __msan_on_report() { FAIL() << "Got MSan error"; }
void __tsan_on_report() { FAIL() << "Got TSan error"; }
}  // extern "C"
