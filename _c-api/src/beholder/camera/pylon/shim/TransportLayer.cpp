// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/camera/pylon/shim/TransportLayer.h"

#include <pylon/CommandParameter.h>
#include <pylon/Container.h>
#include <pylon/Device.h>
#include <pylon/DeviceClass.h>
#include <pylon/DeviceInfo.h>
#include <pylon/TlFactory.h>
#include <pylon/TransportLayer.h>
#include <pylon/TypeMappings.h>
#include <pylon/gige/GigETransportLayer.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "beholder/camera/pylon/shim/Export.h"

namespace beholder {
namespace pylon {
namespace shim {

// TransportLayerImpl handles communication with physical (camera) devices.
class BH_PYLON_SHIM_API TransportLayerImpl : public TransportLayer {
private:
	struct Deleter {
		void operator()(Pylon::ITransportLayer* tl) {
			if (static_cast<bool>(tl)) {
				Pylon::CTlFactory::GetInstance().ReleaseTl(tl);
			}
		}
	};
	using TLPtr = std::unique_ptr<Pylon::ITransportLayer, Deleter>;

	TLPtr tl_;	// underlying transport layer resource
	DeviceClass dc_{DeviceClass::Unknown};	// transport layer device class

protected:
	// Find and create a device with the provided designator.
	[[nodiscard]] Pylon::IPylonDevice*
	createDeviceImpl(const char* serialNumber) const noexcept;

public:
	TransportLayerImpl() = default;

	TransportLayerImpl(const TransportLayerImpl&) = delete;
	TransportLayerImpl(TransportLayerImpl&&) = default;
	TransportLayerImpl& operator=(const TransportLayerImpl&) = delete;
	TransportLayerImpl& operator=(TransportLayerImpl&&) = default;

	~TransportLayerImpl() override = default;

	bool init(DeviceClass dc) noexcept override;

	[[nodiscard]] std::string getFirstSN() const noexcept override;

	[[nodiscard]] Pylon::IPylonDevice*
	createDevice(const char* serialNumber, bool reboot,
				 std::chrono::milliseconds timeout,
				 size_t retries) const noexcept override;
};

Pylon::IPylonDevice*
TransportLayerImpl::createDeviceImpl(const char* serialNumber) const noexcept {
	if (!tl_) {
		std::cerr << "could not create device: "
				  << "transport layer uninitialized" << std::endl;
		return nullptr;
	}
	try {
		Pylon::DeviceInfoList_t devices{};
		if (dc_ == DeviceClass::GigE) {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
			auto* ptr{static_cast<Pylon::IGigETransportLayer*>(tl_.get())};
			ptr->EnumerateAllDevices(devices);
		} else {
			tl_->EnumerateDevices(devices);
		}
		if (devices.empty()) {
			std::cerr << "could not create device: "
					  << "no devices available" << std::endl;
			return nullptr;
		}
		auto selector = [serialNumber](const auto& info) -> bool {
			return std::strcmp(serialNumber, info.GetSerialNumber().c_str()) ==
				   0;
		};
		auto found{std::find_if(devices.begin(), devices.end(), selector)};
		if (found == devices.end()) {
			std::cerr << "could not create device: "
					  << "could not find specified device" << std::endl;
			return nullptr;
		}
		return tl_->CreateDevice(*found);
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not create device: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not create device" << std::endl;
	}
	return nullptr;
}

bool TransportLayerImpl::init(DeviceClass dc) noexcept {
	// a transport layer can only be initialized once
	if (tl_) {
		std::cerr << "transport layer already initialized" << std::endl;
		return false;
	}
	try {
		dc_ = dc;

		Pylon::CTlFactory& factory{Pylon::CTlFactory::GetInstance()};
		switch (dc) {
			case DeviceClass::GigE: {
				tl_.reset(factory.CreateTl(Pylon::BaslerGigEDeviceClass));
				break;
			}
			case DeviceClass::Emulated: {
				tl_.reset(factory.CreateTl(Pylon::BaslerCamEmuDeviceClass));
				break;
			}
			case DeviceClass::Unknown: {
				tl_.reset();
				break;
			}
		}
		return static_cast<bool>(tl_);
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not initialize transport layer: " << e.what()
				  << std::endl;
	} catch (...) {
		std::cerr << "could not initialize transport layer" << std::endl;
	}
	return false;
}

std::string TransportLayerImpl::getFirstSN() const noexcept {
	try {
		Pylon::DeviceInfoList_t devices{};
		if (dc_ == DeviceClass::GigE) {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
			auto* ptr{static_cast<Pylon::IGigETransportLayer*>(tl_.get())};
			ptr->EnumerateAllDevices(devices);
		} else {
			tl_->EnumerateDevices(devices);
		}
		if (devices.empty()) {
			std::cerr << "could not find a device: "
					  << "no devices available" << std::endl;
			return {};
		}
		return devices.front().GetSerialNumber().c_str();
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not find a device: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not find a device" << std::endl;
	}
	return {};
}

Pylon::IPylonDevice*
TransportLayerImpl::createDevice(const char* serialNumber, bool reboot,
								 std::chrono::milliseconds timeout,
								 size_t retries) const noexcept {
	auto* d{createDeviceImpl(serialNumber)};
	if (!reboot || !static_cast<bool>(d)) {
		return d;
	}
	// reboot the device to clear any errors and purge the buffers
	try {
		std::cout << "trying to reset device (S/N): " << serialNumber
				  << std::endl;
		// no try-catch here because if we throw, we have actual issues
		d->Open();
		const bool reset{
			Pylon::CCommandParameter(d->GetNodeMap(), "DeviceReset")
				.TryExecute()};
		// probably unnecessary, but just in case the device is
		// in an invalid state
		tl_->DestroyDevice(d);
		if (reset) {
			std::cout << "waiting for device on-line" << std::endl;
			for (auto i{0UL}; i < retries; ++i) {
				std::this_thread::sleep_for(timeout);
				// FIXME: mute log output here, we only care about failure
				// after all attempts have been made
				d = createDeviceImpl(serialNumber);
				if (static_cast<bool>(d)) {
					return d;
				}
			}
			std::cerr << "could not create device: "
					  << "retry limit reached after reset" << std::endl;
		} else {
			std::cerr << "could not reset device (S/N): " << serialNumber
					  << "; continuing without reset" << std::endl;
			return createDeviceImpl(serialNumber);
		}
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not create device: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not create device" << std::endl;
	}
	return nullptr;
}

}  // namespace shim
}  // namespace pylon
}  // namespace beholder

extern "C" {

BH_PYLON_SHIM_API beholder::pylon::shim::TransportLayer*
pylonTransportLayer_create() {
	return new beholder::pylon::shim::TransportLayerImpl{};
}

BH_PYLON_SHIM_API void
pylonTransportLayer_delete(beholder::pylon::shim::TransportLayer* p) {
	delete p;
}

}  // extern "C"
