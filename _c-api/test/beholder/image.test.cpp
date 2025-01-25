// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Image processing tests.

#include <beholder/image/Processor.h>
#include <gtest/gtest.h>

#include "Testing.h"

namespace beholder {
namespace test {

// Test fixtures and helpers
// -------------------------

// Tests
// -----

// Construct an image processor.
// TODO: figure out what we want to test here
TEST(Processor, Construct) {
	const beholder::Processor proc{};

	EXPECT_TRUE(true);
}

}  // namespace test
}  // namespace beholder
