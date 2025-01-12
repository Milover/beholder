// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// A wrapper class for a GenICam transport layer.
//
#ifndef BEHOLDER_CAMERA_TRANSPORT_LAYER_H
#define BEHOLDER_CAMERA_TRANSPORT_LAYER_H

#include <chrono>
#include <memory>
#include <string>

#include "BeholderCameraExport.h"

namespace Pylon {
class ITransportLayer;
class IPylonDevice;
}  // namespace Pylon

namespace beholder {

// Supported Transport layer types (device classes).
enum class BH_CAM_API DeviceClass : int {
	GigE,
	Emulated,
	//USB,
	Unknown = -1
};

// Supported device designator types, used to select a specific device.
enum class BH_CAM_API DeviceDesignator : int {
	MAC,  // device MAC address
	SN,	  // device serial number
	//IP,			// device IP address
	Unknown = -1
};

// The default timeout for connecting to a camera device.
inline static constexpr std::chrono::milliseconds DfltDevConnTimeout{3000};

// The default number of retry attempts for connecting to a camera device.
inline static constexpr std::size_t DfltDevNRetries{5UL};

// TransportLayer handles communication with physical (camera) devices.
class BH_CAM_API TransportLayer {
private:
	// Deleter is a helper class for releasing the underlying transport layer
	// resources.
	struct Deleter {
		void operator()(Pylon::ITransportLayer* tl);
	};

	// Underlying transport layer.
	std::unique_ptr<Pylon::ITransportLayer, Deleter> tl_{};
	// Class of device supported by the transport layer.
	DeviceClass dc_{DeviceClass::Unknown};

protected:
	// Find and create a device with the provided designator.
	[[nodiscard]] std::unique_ptr<Pylon::IPylonDevice> createDeviceImpl(
		const char* designator,
		DeviceDesignator ddt = DeviceDesignator::SN) const noexcept;

public:
	// Default constructor.
	// The transport layer must be initialized with TransportLayer::init
	// before use.
	TransportLayer() = default;

	TransportLayer(const TransportLayer&) = delete;
	TransportLayer(TransportLayer&&) = default;

	~TransportLayer() = default;

	TransportLayer& operator=(const TransportLayer&) = delete;
	TransportLayer& operator=(TransportLayer&&) = default;

	// Initialize the transport layer for a specific class of devices.
	//
	// Cannot be re-initialized after first initialization.
	// Subsequent calls returnf false.
	bool init(DeviceClass dc = DeviceClass::GigE) noexcept;

	// Find and establish a connection to a device with the provided designator.
	//
	// If reboot is true, the device will be rebooted/reset during creation.
	[[nodiscard]] std::unique_ptr<Pylon::IPylonDevice>
	createDevice(const std::string& designator,
				 DeviceDesignator ddt = DeviceDesignator::SN,
				 bool reboot = true,
				 std::chrono::milliseconds timeout = DfltDevConnTimeout,
				 std::size_t retries = DfltDevNRetries) const noexcept;

	// Find and establish a connection to a device with the provided designator.
	//
	// If reboot is true, the device will be rebooted/reset during creation.
	[[nodiscard]] std::unique_ptr<Pylon::IPylonDevice>
	createDevice(const char* designator,
				 DeviceDesignator ddt = DeviceDesignator::SN,
				 bool reboot = true,
				 std::chrono::milliseconds timeout = DfltDevConnTimeout,
				 std::size_t retries = DfltDevNRetries) const noexcept;

	// Get the serial number of the first device found
	[[nodiscard]] std::string getFirstSN() const noexcept;

#ifndef NDEBUG
	// Return the underlying pointer
	// FIXME: either convert to std::unique_ptr or remove
	[[nodiscard]] Pylon::ITransportLayer* getPtr() const noexcept {
		return tl_.get();
	}
#endif
};

// Return a formatted string of the device designator
[[nodiscard]] BH_CAM_API std::string
formatDeviceDesignator(DeviceDesignator ddt);

}  // namespace beholder

#endif	// BEHOLDER_CAMERA_TRANSPORT_LAYER_H
