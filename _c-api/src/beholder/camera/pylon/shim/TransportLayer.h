// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_PYLONSHIM_TRANSPORT_LAYER_H
#define BEHOLDER_PYLONSHIM_TRANSPORT_LAYER_H

#include <chrono>
#include <string>

#include "beholder/camera/DeviceClass.h"
#include "beholder/camera/pylon/shim/Export.h"

namespace Pylon {
class ITransportLayer;
class IPylonDevice;
}  // namespace Pylon

namespace beholder {
namespace pylonshim {

// TransportLayer handles communication with physical (camera) devices.
class BH_PYLONSHIM_API TransportLayer {
protected:
	// Default constructor.
	// The transport layer must be initialized with TransportLayer::init
	// before use.
	TransportLayer() = default;

public:
	using DeviceClass = ::beholder::camera::DeviceClass;

	TransportLayer(const TransportLayer&) = delete;
	TransportLayer(TransportLayer&&) = default;
	TransportLayer& operator=(const TransportLayer&) = delete;
	TransportLayer& operator=(TransportLayer&&) = default;

	virtual ~TransportLayer() = default;

	// Initialize the transport layer for a specific class of devices.
	//
	// Cannot be re-initialized after first initialization.
	// Subsequent calls returnf false.
	[[nodiscard]] virtual bool init(DeviceClass dc) noexcept = 0;

	// Check if the transport layer has been initialized.
	[[nodiscard]] virtual bool isInitialized() const noexcept = 0;

	// Get the serial number of the first device found
	[[nodiscard]] virtual std::string getFirstSN() const noexcept = 0;

	// Find and establish a connection to a device with the provided designator.
	// If reboot is true, the device will be rebooted/reset during creation.
	[[nodiscard]] virtual Pylon::IPylonDevice*
	createDevice(const char* serialNumber, std::chrono::milliseconds timeout,
				 bool reboot) const noexcept = 0;
};

}  // namespace pylonshim
}  // namespace beholder

#endif	//  BEHOLDER_PYLONSHIM_TRANSPORT_LAYER_H
