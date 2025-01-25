// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Neural network tests.
//
// TODO: drive tests through a JSON config file

#include <beholder/image/Processor.h>
#include <beholder/neural/CRAFTDetector.h>
#include <beholder/neural/EASTDetector.h>
#include <gtest/gtest.h>

#include <exception>
#include <filesystem>

#include "Testing.h"

namespace beholder {
namespace test {

// Test fixtures and helpers
// -------------------------

// Tests
// -----

TEST(Neural, EAST) {  // NOLINT(*-function-cognitive-complexity)
	const auto testimage{assetsDir / "images/test_30px_640x640.png"};
	try {
		// set up detector
		beholder::EASTDetector det{};
		det.modelPath = assetsDir / "models";
		det.model = "east.pb";
		det.size = beholder::EASTDetector::Vec2<>{320, 320};  // NOLINT
		ASSERT_TRUE(det.init());

		// read the image
		Processor proc{};
		ASSERT_TRUE(proc.readImage(testimage, ReadMode::Color));

		// detect text
		EXPECT_TRUE(det.detect(proc.getRawImage()));

		const auto& res{det.getResults()};
		ASSERT_EQ(res.size(), 1);  // FIXME: tie to test image

		// check if the bounding box is within some acceptable region
		// NOTE: this part is acceptance testing mostly
		// TODO: use a comparison function with some tolerance
		const auto& b{res.front().box.cRef()};
		EXPECT_GE(b.left, 270);	   // FIXME: tie to test image
		EXPECT_GE(b.top, 300);	   // FIXME: tie to test image
		EXPECT_LE(b.right, 375);   // FIXME: tie to test image
		EXPECT_LE(b.bottom, 340);  // FIXME: tie to test image
	} catch (const std::exception& e) {
		FAIL() << e.what();
	} catch (...) {
		FAIL() << "caught unknown exception";
	}
}

TEST(Neural, CRAFT) {  // NOLINT(*-function-cognitive-complexity)
	const auto testimage{assetsDir / "images/test_30px_640x640.png"};
	try {
		// set up detector
		CRAFTDetector det{};
		det.modelPath = assetsDir / "models";
		det.model = "craft-320px.onnx";
		det.size = beholder::CRAFTDetector::Vec2<>{320, 320};  // NOLINT
		ASSERT_TRUE(det.init());

		// read the image
		Processor proc{};
		ASSERT_TRUE(proc.readImage(testimage, ReadMode::Color));

		// detect text
		EXPECT_TRUE(det.detect(proc.getRawImage()));

		const auto& res{det.getResults()};
		ASSERT_EQ(res.size(), 1);  // FIXME: tie to test image

		// check if the bounding box is within some acceptable region
		// NOTE: this part is acceptance testing mostly
		// TODO: use a comparison function with some tolerance
		const auto& b{res.front().box.cRef()};
		EXPECT_GE(b.left, 270);	   // FIXME: tie to test image
		EXPECT_GE(b.top, 300);	   // FIXME: tie to test image
		EXPECT_LE(b.right, 375);   // FIXME: tie to test image
		EXPECT_LE(b.bottom, 340);  // FIXME: tie to test image
	} catch (const std::exception& e) {
		FAIL() << e.what();
	} catch (...) {
		FAIL() << "caught unknown exception";
	}
}

}  // namespace test
}  // namespace beholder
