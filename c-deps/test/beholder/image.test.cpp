// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Image processing tests.

#include <beholder/image/Processor.h>
#include <gtest/gtest.h>

#include <array>
#include <bit>
#include <limits>
#include <ostream>
#include <string>
#include <utility>

#include "Testing.h"

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
