/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include <pylon/Container.h>
#include <pylon/Device.h>
#include <pylon/DeviceClass.h>
#include <pylon/DeviceInfo.h>
#include <pylon/ParameterIncludes.h>
#include <pylon/TlFactory.h>
#include <pylon/TransportLayer.h>
#include <pylon/gige/GigETransportLayer.h>

#include "Exception.h"
#include "TransportLayer.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * * //

Pylon::IPylonDevice* TransportLayer::createDeviceImpl
(
	const char* designator,
	DeviceDesignator ddt
) const noexcept
{
	try
	{
		Pylon::DeviceInfoList_t devices {};
		if (auto ptr {dynamic_cast<Pylon::IGigETransportLayer*>(tl_)}; ptr)
		{
			ptr->EnumerateAllDevices(devices);
		}
		else
		{
			tl_->EnumerateDevices(devices);
		}
		if (devices.empty())
		{
			std::cerr << "could not create device: "
					  << "no devices available" << std::endl;
			return nullptr;
		}
		auto selector = [ddt, designator](const auto& info) -> bool
		{
			switch (ddt)
			{
				case DeviceDesignator::MAC:
				{
					return std::strcmp(designator, info.GetMacAddress().c_str()) == 0;
				}
				case DeviceDesignator::SN:
				{
					return std::strcmp(designator, info.GetSerialNumber().c_str()) == 0;
				}
				case DeviceDesignator::Unknown:
				{
					return false;
				}
			}
			return false;
		};
		auto found {std::find_if(devices.begin(), devices.end(), selector)};
		if (found == devices.end())
		{
			std::cerr << "could not create device: "
					  << "could not find specified device" << std::endl;
			return nullptr;
		}
		return tl_->CreateDevice(*found);
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "could not create device: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "could not create device" << std::endl;
	}
	return nullptr;
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

TransportLayer::~TransportLayer()
{
	if (tl_)
	{
		Pylon::CTlFactory::GetInstance().ReleaseTl(tl_);
		tl_ = nullptr;
	}
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool TransportLayer::init(DeviceClass dc) noexcept
{
	try
	{
		Pylon::CTlFactory& factory {Pylon::CTlFactory::GetInstance()};
		switch (dc)
		{
			case DeviceClass::GigE:
			{
				tl_ = factory.CreateTl(Pylon::BaslerGigEDeviceClass);
				return true;
				break;
			}
			case DeviceClass::Unknown:
			{
				tl_ = nullptr;
				return false;
				break;
			}
		}
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "could not initialize transport layer: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "could not initialize transport layer" << std::endl;
	}
	return false;
}

Pylon::IPylonDevice* TransportLayer::createDevice
(
	const std::string& designator,
	DeviceDesignator ddt
) const noexcept
{
	return createDevice(designator.c_str(), ddt);
}


Pylon::IPylonDevice* TransportLayer::createDevice
(
	const char* designator,
	DeviceDesignator ddt
) const noexcept
{
	auto d {createDeviceImpl(designator, ddt)};
	if (!d)
	{
		return d;
	}
	// reboot the device to clear any errors and purge the buffers
	try
	{
		std::cout << "trying to reset device (" << designator << ")" << std::endl;
		if (!Pylon::CCommandParameter(d->GetNodeMap(), "DeviceReset").TryExecute())
		{
			std::cerr << "could not reset device (" << designator << "); "
					  << "continuing without reset" << std::endl;
			return d;
		}
		tl_->DestroyDevice(d);
		d = nullptr;

		// FIXME: 'retries' and 'waitTime' should be adjustable
		std::cout << "waiting for device on-line" << std::endl;
		auto waitTime {std::chrono::seconds{3}};
		auto nRetries {5ul};
		for (auto i {0ul}; i < nRetries; ++i)
		{
			std::this_thread::sleep_for(waitTime);
			if ((d = createDeviceImpl(designator, ddt)))
			{
				return d;
			}
		}
		std::cerr << "could not create device: "
				  << "retry limit reached after reset" << std::endl;
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "could not create device: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "could not create device" << std::endl;
	}
	return nullptr;
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
