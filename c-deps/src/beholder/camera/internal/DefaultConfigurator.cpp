// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/camera/internal/DefaultConfigurator.h"

#include <GenApi/INodeMap.h>
#include <pylon/ConfigurationHelper.h>
#include <pylon/InstantCamera.h>
#include <pylon/ParameterIncludes.h>

#include <array>
#include <iostream>
#include <stdexcept>

#include "beholder/camera/Exception.h"

namespace beholder {
namespace internal {

void DefaultConfigurator::applyConfiguration(GenApi::INodeMap& nodemap) const {
	using Cmd = Pylon::CCommandParameter;
	using Enum = Pylon::CEnumParameter;
	using Int = Pylon::CIntegerParameter;
	using Cfg = Pylon::CConfigurationHelper;

	// load default set
	Enum{nodemap, "UserSetSelector"}.SetValue("Default");
	Cmd{nodemap, "UserSetLoad"}.Execute();

	// disable all triggers, image compression and streaming
	Cfg::DisableAllTriggers(nodemap);
	Cfg::DisableCompression(nodemap);
	Cfg::DisableGenDC(nodemap);

	// reset image ROI
	if (!Int{nodemap, "Width"}.TrySetToMaximum() ||
		!Int{nodemap, "Height"}.TrySetToMaximum() ||
		!Int{nodemap, "OffsetX"}.TrySetToMinimum() ||
		!Int{nodemap, "OffsetY"}.TrySetToMinimum()) {
		std::cerr << "could not reset image ROI" << std::endl;
	}
	// set preferred pixel format (8-bit single-channel)
	std::array<const char*, 2> formats{{"BayerRG8", "Mono8"}};
	if (!Enum{nodemap, "PixelFormat"}.TrySetValue(formats.data())) {
		std::cerr << "could not set default pixel format" << std::endl;
	}
	// see comments on Camera::isAcquiring() for more info
	std::array<const char*, 1> modes{{"Continuous"}};
	if (!Enum{nodemap, "AcquisitionMode"}.TrySetValue(modes.data())) {
		std::cerr << "could not set continuous acquisition mode" << std::endl;
	}
}

void DefaultConfigurator::OnOpened(Pylon::CInstantCamera& cam) {
	try {
		applyConfiguration(cam.GetNodeMap());
		// Probe max packet size
		Pylon::CConfigurationHelper::ProbePacketSize(
			cam.GetStreamGrabberNodeMap());
	} catch (const Pylon::GenericException& e) {
		throw Exception{"could not apply 'Default' configuration: ", e};
	} catch (const std::exception& e) {
		throw Exception{"could not apply 'Default' configuration: ", e};
	} catch (...) {
		throw Exception{"could not apply 'Default' configuration"};
	}
}

}  // namespace internal
}  // namespace beholder
