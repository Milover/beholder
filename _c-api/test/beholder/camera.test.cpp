// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Camera API tests.

#include <beholder/camera/Camera.h>
#include <beholder/camera/ParamEntry.h>
#include <beholder/camera/PylonAPI.h>
#include <beholder/camera/TransportLayer.h>
#include <beholder/capi/Image.h>
#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <string>

#include "Testing.h"  // NOLINT

namespace beholder {
namespace test {

// Test fixtures and helpers
// -------------------------

// Tests
// -----
//
// NOTE: camera emulation is enabled for all tests by default through CMake
// defined environment variables, and 3 emulated devices are available.
// However, defining the environment within the test or fixture might be
// more appropriate.

// Connect to an emulated camera device and acquire an image.
//
// BUG: when compiling with clang and running with sanitizers, this test
// can occasionally hang --- the camera times out while waiting for the trigger
// to become available.
// This only happens on the first run after a fresh build.
TEST(CameraEmulated, AcquireImage) {  // NOLINT(*-function-cognitive-complexity)
	const auto testimage{globalAssetsDir / "images/red_100x100.png"};
	const ParamList camParams{
		ParamEntry{"AcquisitionMode", "Continuous"},

		ParamEntry{"TriggerSelector", "FrameStart"},
		ParamEntry{"TriggerMode", "On"},
		ParamEntry{"TriggerSource", "Software"},

		ParamEntry{"TestImageSelector", "Off"},
		ParamEntry{"ImageFileMode", "On"},
		ParamEntry{"ImageFilename", testimage},
	};
	const std::string sn{"0815-0000"};	// emulated camera SN
	const std::size_t nImages{3};		// No. images to acquire

	// before using any pylon methods, the pylon runtime must be initialized.
	const PylonAPI api{};

	try {
		// create transport layer
		TransportLayer tl{};
		ASSERT_TRUE(tl.init(DeviceClass::Emulated));

		// create device
		auto* dev{tl.createDevice(sn.c_str(), DeviceDesignator::SN)};
		ASSERT_NE(dev, nullptr);

		// create camera and apply configuration
		Camera cam{};
		ASSERT_TRUE(cam.init(dev));
		ASSERT_TRUE(cam.isInitialized());
		EXPECT_TRUE(cam.setParams(camParams));
		//dumpParams(cam.getParams(ParamAccessMode::Read));

		// acquire image(s)
		ASSERT_TRUE(cam.startAcquisition(nImages));

		for (auto i{0UL}; i < nImages; ++i) {
			EXPECT_TRUE(cam.waitAndTrigger(std::chrono::seconds{30}));
			EXPECT_TRUE(cam.acquire());

			auto img{cam.getImage()};
			ASSERT_TRUE(img.has_value());
			EXPECT_EQ(img->cRef().rows, 100);  // NOLINT(*-optional-access)
			EXPECT_EQ(img->cRef().cols, 100);  // NOLINT(*-optional-access)
		}
	} catch (...) {
		FAIL();
	}
}

}  // namespace test
}  // namespace beholder
