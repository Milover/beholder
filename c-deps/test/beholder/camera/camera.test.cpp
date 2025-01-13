// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Image processing unit tests.

#include "beholder/camera/Camera.h"

#include <gtest/gtest.h>
#include <stdlib.h>	 // NOLINT(modernize-*): needed for setenv

#include <array>
#include <chrono>
#include <filesystem>
#include <limits>
#include <ostream>
#include <source_location>
#include <string>

#include "beholder/camera/ParamEntry.h"
#include "beholder/camera/PylonAPI.h"
#include "beholder/camera/TransportLayer.h"
#include "beholder/capi/Image.h"
#include "beholder/image/Processor.h"

// Report sanitizer errors.
extern "C" void __ubsan_on_report() { FAIL() << "Got UBSan error"; }  // NOLINT
extern "C" void __asan_on_report() { FAIL() << "Got ASan error"; }	  // NOLINT
extern "C" void __msan_on_report() { FAIL() << "Got MSan error"; }	  // NOLINT
extern "C" void __tsan_on_report() { FAIL() << "Got TSan error"; }	  // NOLINT

namespace beholder {
namespace test {

// Test fixtures and helpers
// -------------------------

// Tests
// -----

// Run tests defined in params using eqFinal as the comparison function.
TEST(Camera, AcquireImage) {  // NOLINT(*-function-cognitive-complexity)
	const auto testimage{
		std::filesystem::absolute(std::source_location::current().file_name())
			.parent_path()
			.parent_path() /
		"testdata/red_100x100.png"};
	const ParamList camParams{
		ParamEntry{"AcquisitionMode", "Continuous"},

		ParamEntry{"TriggerSelector", "FrameStart"},
		ParamEntry{"TriggerMode", "On"},
		ParamEntry{"TriggerSource", "Software"},

		ParamEntry{"TestImageSelector", "Off"},
		ParamEntry{"ImageFileMode", "On"},
		ParamEntry{"ImageFilename", testimage},
	};
	constexpr std::string_view sn{"0815-0000"};	 // emulated camera SN
	constexpr std::size_t nImages{1};			 // No. images to acquire

	// set to enable camera emulation
	setenv("PYLON_CAMEMU", "1", static_cast<int>(true));
	// before using any pylon methods, the pylon runtime must be initialized.
	const PylonAPI api{};

	try {
		// create transport layer
		TransportLayer tl{};
		ASSERT_TRUE(tl.init(DeviceClass::Emulated));

		// create device
		auto* dev{tl.createDevice(sn.data(), DeviceDesignator::SN)};
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
			EXPECT_TRUE(cam.waitAndTrigger());
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
