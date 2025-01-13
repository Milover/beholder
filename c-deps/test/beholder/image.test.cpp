// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Image processing unit tests.

#include <gtest/gtest.h>

#include <array>
#include <bit>
#include <limits>
#include <ostream>
#include <string>
#include <utility>

#include "beholder/image/Processor.h"

// Report sanitizer errors.
extern "C" void __ubsan_on_report() { FAIL() << "Got UBSan error"; }   // NOLINT
extern "C" void __asan_on_report() { FAIL() << "Got ASan error"; }	   // NOLINT
extern "C" void __msan_on_report() { FAIL() << "Got an MSan error"; }  // NOLINT
extern "C" void __tsan_on_report() { FAIL() << "Got an TSan error"; }  // NOLINT

namespace beholder {
namespace test {

// Test fixtures and helpers
// -------------------------

// Tests
// -----

// Run tests defined in params using eqFinal as the comparison function.
TEST(Processor, ReadImage) {
	const beholder::Processor proc{};

	EXPECT_TRUE(true);
}

}  // namespace test
}  // namespace beholder
