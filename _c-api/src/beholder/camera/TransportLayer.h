// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// A wrapper class for a GenICam transport layer.

#ifndef BEHOLDER_CAMERA_TRANSPORT_LAYER_H
#define BEHOLDER_CAMERA_TRANSPORT_LAYER_H

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>

#include "beholder/camera/Backends.h"

namespace beholder {
namespace camera {

class TransportLayerInterface;
using TLPtr = std::unique_ptr<TransportLayerInterface>;
using Milliseconds = std::chrono::milliseconds;

// The default timeout for connecting to a camera device.
static constexpr Milliseconds DfltDevConnTimeout{3000};

// The default number of retry attempts for connecting to a camera device.
static constexpr size_t DfltDevNRetries{5UL};

// Supported Transport layer types (device classes).
enum class DeviceClass {
	Unknown = -1,
	GigE,
	Emulated,
	//USB,
};

// Supported device designator types, used to select a specific device.
enum class DeviceDesignator {
	Unknown = -1,
	MAC,  // device MAC address
	SN,	  // device serial number
	//IP,	  // device IP address
};

class TransportLayerInterface {
private:
	TransportLayerInterface() = default;

public:
	TransportLayerInterface(const TransportLayerInterface&) = delete;
	TransportLayerInterface(TransportLayerInterface&&) = delete;
	TransportLayerInterface& operator=(const TransportLayerInterface&) = delete;
	TransportLayerInterface& operator=(TransportLayerInterface&&) = delete;
	virtual ~TransportLayerInterface() = default;

	[[nodiscard]] virtual bool init(DeviceClass dc) noexcept = 0;

	[[nodiscard]] virtual void* createDevice(const std::string& designator,
											 DeviceDesignator ddt, bool reboot,
											 Milliseconds timeout,
											 size_t retries) const noexcept = 0;
	[[nodiscard]] virtual void*
	createDevice(const char* designator, DeviceDesignator ddt, bool reboot,
				 Milliseconds timeout, size_t retries) const noexcept = 0;

	[[nodiscard]] virtual std::string getFirstSN() const noexcept = 0;
};

// TransportLayer handles communication with physical (camera) devices.
class TransportLayer {
private:
	// The underlying implementation.
	TLPtr impl_;

public:
	// Default constructor.
	// The transport layer must be initialized with TransportLayer::init
	// before use.
	explicit TransportLayer(Backend b = Backend::Pylon)
		: impl_{createTransportLayer(b)} {}

	// Initialize the transport layer for a specific class of devices.
	//
	// Cannot be re-initialized after first initialization.
	// Subsequent calls returnf false.
	[[nodiscard]] bool init(DeviceClass dc = DeviceClass::GigE) noexcept {
		return impl_->init(dc);
	}

	// Find and establish a connection to a device with the provided designator.
	// If reboot is true, the device will be rebooted/reset during creation.
	//
	// NOTE: we specifically use a raw pointer instead of a unique_ptr because
	// using it would require including pylon headers at the call site,
	// which we want to avoid.
	//
	// TODO: we should find a way of avoiding exposing this pointer, because
	// it has to be cleaned up with pylon API calls, which will cause problems.
	[[nodiscard]] void*
	createDevice(const std::string& designator,
				 DeviceDesignator ddt = DeviceDesignator::SN,
				 bool reboot = true, Milliseconds timeout = DfltDevConnTimeout,
				 size_t retries = DfltDevNRetries) const noexcept {
		return impl_->createDevice(designator, ddt, reboot, timeout, retries);
	}

	// Find and establish a connection to a device with the provided designator.
	// If reboot is true, the device will be rebooted/reset during creation.
	//
	// NOTE: we specifically use a raw pointer instead of a unique_ptr because
	// using it would require including pylon headers at the call site,
	// which we want to avoid.
	//
	// TODO: we should find a way of avoiding exposing this pointer, because
	// it has to be cleaned up with pylon API calls, which will cause problems.
	[[nodiscard]] void*
	createDevice(const char* designator,
				 DeviceDesignator ddt = DeviceDesignator::SN,
				 bool reboot = true, Milliseconds timeout = DfltDevConnTimeout,
				 size_t retries = DfltDevNRetries) const noexcept {
		return impl_->createDevice(designator, ddt, reboot, timeout, retries);
	}

	// Get the serial number of the first device found
	[[nodiscard]] std::string getFirstSN() const noexcept {
		return impl_->getFirstSN();
	}
};

// Return a formatted string of the device designator
[[nodiscard]] std::string formatDeviceDesignator(DeviceDesignator ddt);

}  // namespace camera
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_TRANSPORT_LAYER_H
