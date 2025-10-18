// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_CAMERA_CONFIG_H
#define BEHOLDER_CAMERA_CONFIG_H

#include <chrono>
#include <string>

#include "beholder/camera/Backends.h"
#include "beholder/camera/DeviceClass.h"

namespace beholder {
namespace camera {

using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;

// Config holds camera configuration parameters.
struct Config {
	// The default timeout for acquiring an image.
	static constexpr Milliseconds DfltAcquisitionTimeout{1000};
	// The default trigger timeout.
	static constexpr Milliseconds DfltTriggerTimeout{100};
	// The default timeout used when connecting to a camera device during
	// initialization.
	static constexpr Seconds DfltConnectionTimeout{15};

	// Camera backend.
	Backend backend{Backend::Pylon};
	// The camera device transport layer type.
	DeviceClass deviceClass{DeviceClass::GigE};
	// Device designator (usually device S/N).
	std::string designator;
	// Image acquisition timeout.
	Milliseconds acquisitionTimeout{DfltAcquisitionTimeout};
	// Trigger timeout.
	Milliseconds triggerTimeout{DfltTriggerTimeout};
	// Timeout used when attempting to connect to the camera device.
	Seconds connectionTimeout{DfltConnectionTimeout};
	// Reboot the camera device when connection is established.
	bool rebootOnConnection{true};
};

}  // namespace camera
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_CONFIG_H
