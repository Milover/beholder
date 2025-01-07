// Copyright © 2024 P. Milovic
// SPDX-License-Identifier: MIT

// Compile and basic operation test for the pylon library.

#include <gtest/gtest.h>
#include <pylon/PylonIncludes.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>

namespace py = Pylon;

// Test fixtures and helpers
// -------------------------

class TestConfig : public py::CConfigurationEventHandler {
protected:
	using Helper = py::CConfigurationHelper;

	// Apply the configuration to the camera device.
	virtual void applyConfiguration(GenApi::INodeMap& nodemap) const {
		using Enum = py::CEnumParameter;
		using Cmd = py::CCommandParameter;
		using Int = py::CIntegerParameter;

		// load default set
		Enum{nodemap, "UserSetSelector"}.SetValue("Default");
		Cmd{nodemap, "UserSetLoad"}.Execute();

		// disable all triggers, image compression and streaming
		Helper::DisableAllTriggers(nodemap);
		Helper::DisableCompression(nodemap);
		Helper::DisableGenDC(nodemap);

		// set preferred pixel format (8-bit single-channel)
		Enum{nodemap, "PixelFormat"}.SetValue("BayerRG8");

		// reset image ROI
		Int{nodemap, "Width"}.TrySetToMaximum();
		Int{nodemap, "Height"}.TrySetToMaximum();
		Int{nodemap, "OffsetX"}.TrySetToMinimum();
		Int{nodemap, "OffsetY"}.TrySetToMinimum();

		// set up acquisition mode and triggering
		Enum{nodemap, "AcquisitionMode"}.SetValue("Continuous");
		Enum{nodemap, "TriggerSelector"}.SetValue("FrameStart");
		Enum{nodemap, "TriggerMode"}.SetValue("On");
		Enum{nodemap, "TriggerSource"}.SetValue("Software");
	}

public:
	TestConfig() = default;
	TestConfig(const TestConfig&) = default;
	TestConfig(TestConfig&&) = default;
	TestConfig& operator=(const TestConfig&) = default;
	TestConfig& operator=(TestConfig&&) = default;
	~TestConfig() override = default;

	// Apply configuration right after the camera device is opened.
	// Throws on error.
	void OnOpened(py::CInstantCamera& cam) override {
		applyConfiguration(cam.GetNodeMap());
		Helper::ProbePacketSize(cam.GetStreamGrabberNodeMap());
	}
};

// Tests
// -----

// Attach to an emulated camera device and grab a test image, provided
// from a local file.
TEST(pylon, CamemuImgGrab) {  // NOLINT
	using Factory = py::CTlFactory;
	using TL = py::ITransportLayer;

	// set test params
	constexpr int nCams{1};
	constexpr int nImgs{1};
	constexpr std::chrono::milliseconds timeout{1000};
	constexpr std::string_view expectedSN{"0815-0000"};

	// set up camera emulation with nCams cameras
	setenv("PYLON_CAMEMU", std::to_string(nCams).c_str(),
		   static_cast<int>(true));

	try {
		// set up transport layer
		const py::PylonAutoInitTerm autoInitTerm;

		auto del = [](TL* ptr) { Factory::GetInstance().ReleaseTl(ptr); };
		std::unique_ptr<TL, decltype(del)> tl{
			Factory::GetInstance().CreateTl(py::BaslerCamEmuDeviceClass), del};
		ASSERT_NE(tl, nullptr);

		// discover camera
		py::DeviceInfoList_t devices;
		tl->EnumerateDevices(devices);
		ASSERT_EQ(devices.size(), nCams);

		// check camera SN
		auto sn{devices.front().GetSerialNumber()};
		ASSERT_STREQ(sn.c_str(), expectedSN.data());

		// set up the camera
		py::CInstantCamera cam{};
		cam.RegisterConfiguration(new TestConfig{},	 // NOLINT
								  py::RegistrationMode_ReplaceAll,
								  py::Cleanup_Delete);

		// attach to and open the camera device
		cam.Attach(tl->CreateDevice(devices.front()), py::Cleanup_Delete);
		ASSERT_TRUE(cam.IsPylonDeviceAttached());
		cam.Open();
		ASSERT_TRUE(cam.IsOpen());
		ASSERT_FALSE(cam.IsCameraDeviceRemoved());

		// grab a test image
		py::CGrabResultPtr res{};

		cam.StartGrabbing(nImgs);
		ASSERT_TRUE(cam.IsGrabbing());

		cam.WaitForFrameTriggerReady(timeout.count());
		cam.ExecuteSoftwareTrigger();
		cam.RetrieveResult(timeout.count(), res);
		EXPECT_TRUE(res.IsValid());
		EXPECT_TRUE(res->GrabSucceeded());
		//EXPECT_FALSE(cam.IsGrabbing());
	} catch (const py::GenericException& e) {
		std::cerr << "caught an exception: " << e.what() << std::endl;
		FAIL();
	}
}
