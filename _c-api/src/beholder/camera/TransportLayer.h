// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// A wrapper class for a GenICam transport layer.

#ifndef BEHOLDER_CAMERA_TRANSPORT_LAYER_H
#define BEHOLDER_CAMERA_TRANSPORT_LAYER_H

#include <chrono>
#include <memory>
#include <string>

#include "beholder/BeholderExport.h"

namespace Pylon {
class ITransportLayer;
class IPylonDevice;
}  // namespace Pylon

namespace beholder {

// Supported Transport layer types (device classes).
enum class BH_API DeviceClass {
	GigE,
	Emulated,
	//USB,
	Unknown = -1
};

// Supported device designator types, used to select a specific device.
enum class BH_API DeviceDesignator {
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
class BH_API TransportLayer {
private:
	// Deleter is a helper class for releasing the underlying transport layer
	// resources.
	struct Deleter {
		void operator()(Pylon::ITransportLayer* tl);
	};

	// Underlying transport layer.
	std::unique_ptr<Pylon::ITransportLayer, Deleter> tl_;
	// Class of device supported by the transport layer.
	DeviceClass dc_{DeviceClass::Unknown};

protected:
	// Find and create a device with the provided designator.
	[[nodiscard]] Pylon::IPylonDevice* createDeviceImpl(
		const char* designator,
		DeviceDesignator ddt = DeviceDesignator::SN) const noexcept;

public:
	// Default constructor.
	// The transport layer must be initialized with TransportLayer::init
	// before use.
	TransportLayer() = default;

	TransportLayer(const TransportLayer&) = delete;
	TransportLayer(TransportLayer&&) = delete;

	// Default destructor.
	// Defined in the source because unique_ptr complains about
	// incomplete types.
	~TransportLayer();

	TransportLayer& operator=(const TransportLayer&) = delete;
	TransportLayer& operator=(TransportLayer&&) = delete;

	// Initialize the transport layer for a specific class of devices.
	//
	// Cannot be re-initialized after first initialization.
	// Subsequent calls returnf false.
	bool init(DeviceClass dc = DeviceClass::GigE) noexcept;

	// Find and establish a connection to a device with the provided designator.
	// If reboot is true, the device will be rebooted/reset during creation.
	//
	// NOTE: we specifically use a raw pointer instead of a unique_ptr because
	// using it would require including pylon headers at the call site,
	// which we want to avoid.
	//
	// TODO: we should find a way of avoiding exposing this pointer, because
	// it has to be cleaned up with pylon API calls, which will cause problems.
	[[nodiscard]] Pylon::IPylonDevice*
	createDevice(const std::string& designator,
				 DeviceDesignator ddt = DeviceDesignator::SN,
				 bool reboot = true,
				 std::chrono::milliseconds timeout = DfltDevConnTimeout,
				 std::size_t retries = DfltDevNRetries) const noexcept;

	// Find and establish a connection to a device with the provided designator.
	// If reboot is true, the device will be rebooted/reset during creation.
	//
	// NOTE: we specifically use a raw pointer instead of a unique_ptr because
	// using it would require including pylon headers at the call site,
	// which we want to avoid.
	//
	// TODO: we should find a way of avoiding exposing this pointer, because
	// it has to be cleaned up with pylon API calls, which will cause problems.
	[[nodiscard]] Pylon::IPylonDevice*
	createDevice(const char* designator,
				 DeviceDesignator ddt = DeviceDesignator::SN,
				 bool reboot = true,
				 std::chrono::milliseconds timeout = DfltDevConnTimeout,
				 std::size_t retries = DfltDevNRetries) const noexcept;

	// Get the serial number of the first device found
	[[nodiscard]] std::string getFirstSN() const noexcept;
};

// Return a formatted string of the device designator
[[nodiscard]] BH_API std::string formatDeviceDesignator(DeviceDesignator ddt);

}  // namespace beholder

#endif	// BEHOLDER_CAMERA_TRANSPORT_LAYER_H
