// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_PYLON_SHIM_TRANSPORT_LAYER_H
#define BEHOLDER_PYLON_SHIM_TRANSPORT_LAYER_H

#include <chrono>
#include <cstddef>
#include <string>

#include "beholder/camera/pylon/shim/Export.h"

namespace Pylon {
class ITransportLayer;
class IPylonDevice;
}  // namespace Pylon

namespace beholder {
namespace pylon {
namespace shim {

// TransportLayer handles communication with physical (camera) devices.
class BH_PYLON_SHIM_API TransportLayer {
protected:
	// Default constructor.
	// The transport layer must be initialized with TransportLayer::init
	// before use.
	TransportLayer() = default;

public:
	// Supported transport layer types (device classes).
	enum class DeviceClass {
		Unknown = -1,
		GigE,
		Emulated,
		//USB,
	};

	TransportLayer(const TransportLayer&) = delete;
	TransportLayer(TransportLayer&&) = default;
	TransportLayer& operator=(const TransportLayer&) = delete;
	TransportLayer& operator=(TransportLayer&&) = default;

	virtual ~TransportLayer() = default;

	// Initialize the transport layer for a specific class of devices.
	//
	// Cannot be re-initialized after first initialization.
	// Subsequent calls returnf false.
	virtual bool init(DeviceClass dc) noexcept = 0;

	// Get the serial number of the first device found
	[[nodiscard]] virtual std::string getFirstSN() const noexcept = 0;

	// Find and establish a connection to a device with the provided designator.
	// If reboot is true, the device will be rebooted/reset during creation.
	//
	// NOTE: we specifically use a raw pointer instead of a unique_ptr because
	// using it would require including pylon headers at the call site,
	// which we want to avoid.
	//
	// TODO: we should find a way of avoiding exposing this pointer, because
	// it has to be cleaned up with pylon API calls, which will cause problems.
	[[nodiscard]] virtual Pylon::IPylonDevice*
	createDevice(const char* serialNumber, bool reboot,
				 std::chrono::milliseconds timeout,
				 size_t retries) const noexcept = 0;
};

}  // namespace shim
}  // namespace pylon
}  // namespace beholder

#endif	//  BEHOLDER_PYLON_SHIM_TRANSPORT_LAYER_H
