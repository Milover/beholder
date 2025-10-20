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
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>

#include "beholder/camera/pylon/shim/Export.h"
#include "beholder/util/Enums.h"

namespace chr = std::chrono;

namespace beholder {
namespace pylonshim {

// Polling interval when attempting to create a device.
static constexpr chr::milliseconds DfltConnectionPollInterval{250};

// TransportLayerImpl handles communication with physical (camera) devices.
class BH_PYLONSHIM_API TransportLayerImpl : public TransportLayer {
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

	[[nodiscard]] bool init(DeviceClass dc) noexcept override;
	[[nodiscard]] bool isInitialized() const noexcept override;

	[[nodiscard]] std::string getFirstSN() const noexcept override;

	[[nodiscard]] Pylon::IPylonDevice*
	createDevice(const char* serialNumber, chr::milliseconds timeout,
				 bool reboot) const noexcept override;
};

Pylon::IPylonDevice*
TransportLayerImpl::createDeviceImpl(const char* serialNumber) const noexcept {
	assert(isInitialized() == true);
	auto printErr = [](DeviceClass dc, std::string_view msg) {
		std::cerr << "could not find a device: "
				  << "(device-class: " << enums::to(dc) << "): " << msg
				  << std::endl;
	};
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
			printErr(dc_, "no devices available");
			return nullptr;
		}
		auto selector = [serialNumber](const auto& info) -> bool {
			return std::strcmp(serialNumber, info.GetSerialNumber().c_str()) ==
				   0;
		};
		auto found{std::find_if(devices.begin(), devices.end(), selector)};
		if (found == devices.end()) {
			printErr(dc_, "could not find specified device");
			return nullptr;
		}
		return tl_->CreateDevice(*found);
	} catch (const Pylon::GenericException& e) {
		printErr(dc_, e.what());
	} catch (...) {
		printErr(dc_, "");
	}
	return nullptr;
}

bool TransportLayerImpl::init(DeviceClass dc) noexcept {
	assert(isInitialized() == false);
	try {
		dc_ = dc;
		Pylon::CTlFactory& factory{Pylon::CTlFactory::GetInstance()};
		switch (dc_) {
			case DeviceClass::GigE: {
				tl_.reset(factory.CreateTl(Pylon::BaslerGigEDeviceClass));
				break;
			}
			case DeviceClass::Emulated: {
				tl_.reset(factory.CreateTl(Pylon::BaslerCamEmuDeviceClass));
				break;
			}
			default: {
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

bool TransportLayerImpl::isInitialized() const noexcept {
	return static_cast<bool>(tl_);
}

std::string TransportLayerImpl::getFirstSN() const noexcept {
	auto printErr = [](DeviceClass dc, std::string_view msg) {
		std::cerr << "could not find a device: "
				  << "(device-class: " << enums::to(dc) << "): " << msg
				  << std::endl;
	};
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
			printErr(dc_, "no devices available");
			return {};
		}
		return devices.front().GetSerialNumber().c_str();
	} catch (const Pylon::GenericException& e) {
		printErr(dc_, e.what());
	} catch (...) {
		printErr(dc_, "");
	}
	return {};
}

Pylon::IPylonDevice*
TransportLayerImpl::createDevice(const char* serialNumber,
								 chr::milliseconds timeout,
								 bool reboot) const noexcept {
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

		const auto start{chr::high_resolution_clock::now()};
		auto timedOut = [&start, &timeout]() -> bool {
			const auto now{chr::high_resolution_clock::now()};
			const auto elapsed{
				chr::duration_cast<chr::milliseconds>(now - start)};
			return elapsed < timeout;
		};
		if (reset) {
			std::cout << "waiting for device on-line" << std::endl;
			while (!timedOut()) {
				d = createDeviceImpl(serialNumber);
				if (static_cast<bool>(d)) {
					return d;
				}
				std::this_thread::sleep_for(DfltConnectionPollInterval);
			};
			std::cerr << "could not create device: "
					  << "connection timeout reached after reset" << std::endl;
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

}  // namespace pylonshim
}  // namespace beholder

extern "C" {

BH_PYLONSHIM_API beholder::pylonshim::TransportLayer*
pylonTransportLayer_create() {
	return new beholder::pylonshim::TransportLayerImpl{};
}

BH_PYLONSHIM_API void
pylonTransportLayer_delete(beholder::pylonshim::TransportLayer* p) {
	delete p;
}

}  // extern "C"
