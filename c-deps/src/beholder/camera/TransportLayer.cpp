// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/camera/TransportLayer.h"

#include <pylon/Container.h>
#include <pylon/Device.h>
#include <pylon/DeviceClass.h>
#include <pylon/DeviceInfo.h>
#include <pylon/ParameterIncludes.h>
#include <pylon/TlFactory.h>
#include <pylon/TransportLayer.h>
#include <pylon/gige/GigETransportLayer.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace beholder {

void TransportLayer::Deleter::operator()(Pylon::ITransportLayer* tl) {
	if (static_cast<bool>(tl)) {
		Pylon::CTlFactory::GetInstance().ReleaseTl(tl);
	}
}

Pylon::IPylonDevice*
TransportLayer::createDeviceImpl(const char* designator,
								 DeviceDesignator ddt) const noexcept {
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
		auto selector = [ddt, designator](const auto& info) -> bool {
			switch (ddt) {
				case DeviceDesignator::MAC: {
					return std::strcmp(designator,
									   info.GetMacAddress().c_str()) == 0;
				}
				case DeviceDesignator::SN: {
					return std::strcmp(designator,
									   info.GetSerialNumber().c_str()) == 0;
				}
				case DeviceDesignator::Unknown: {
					return false;
				}
			}
			return false;
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

// NOLINTNEXTLINE(*-use-equals-default): incomplete type; must be defined here
TransportLayer::~TransportLayer(){};

bool TransportLayer::init(DeviceClass dc) noexcept {
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

Pylon::IPylonDevice*
TransportLayer::createDevice(const std::string& designator,
							 DeviceDesignator ddt, bool reboot,
							 std::chrono::milliseconds timeout,
							 std::size_t retries) const noexcept {
	return createDevice(designator.c_str(), ddt, reboot, timeout, retries);
}

Pylon::IPylonDevice*
TransportLayer::createDevice(const char* designator, DeviceDesignator ddt,
							 bool reboot, std::chrono::milliseconds timeout,
							 std::size_t retries) const noexcept {
	auto d{createDeviceImpl(designator, ddt)};
	if (!reboot || !d) {
		return d;
	}
	// reboot the device to clear any errors and purge the buffers
	try {
		std::cout << "trying to reset device: " << formatDeviceDesignator(ddt)
				  << " : " << designator << std::endl;
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
				if (d = createDeviceImpl(designator, ddt); d) {
					return d;
				}
			}
			std::cerr << "could not create device: "
					  << "retry limit reached after reset" << std::endl;
		} else {
			std::cerr << "could not reset device: "
					  << formatDeviceDesignator(ddt) << " : " << designator
					  << "; continuing without reset" << std::endl;
			return createDeviceImpl(designator, ddt);
		}
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not create device: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not create device" << std::endl;
	}
	return nullptr;
}

std::string TransportLayer::getFirstSN() const noexcept {
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

std::string formatDeviceDesignator(DeviceDesignator ddt) {
	switch (ddt) {
		case DeviceDesignator::MAC:
			return "MAC";
		case DeviceDesignator::SN:
			return "S/N";
		case DeviceDesignator::Unknown:
			return "unknown";
	}
	return "unknown";
}

}  // namespace beholder
