/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cassert>
#include <string>

#include <pylon/Container.h>
#include <pylon/Device.h>
#include <pylon/DeviceClass.h>
#include <pylon/DeviceInfo.h>
#include <pylon/TlFactory.h>
#include <pylon/TransportLayer.h>
#include <pylon/gige/GigETransportLayer.h>

#include "Exception.h"
#include "TransportLayer.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

TransportLayer::TransportLayer(DeviceClass dc)
{
	Pylon::CTlFactory& factory {Pylon::CTlFactory::GetInstance()};
	switch (dc)
	{
		case DeviceClass::GigE:
		{
			tl_ = factory.CreateTl(Pylon::BaslerGigEDeviceClass);
			break;
		}
		case DeviceClass::Unknown:
		{
			tl_ = nullptr;
			break;
		}
	}
	assert(tl_ != nullptr);
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

TransportLayer::~TransportLayer()
{
	Pylon::CTlFactory::GetInstance().ReleaseTl(tl_);
	tl_ = nullptr;
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Pylon::IPylonDevice* TransportLayer::createDevice
(
	const std::string& designator,
	DeviceDesignator ddt
) const
{
	return createDevice(designator.c_str(), ddt);
}


Pylon::IPylonDevice* TransportLayer::createDevice
(
	const char* designator,
	DeviceDesignator ddt
) const
{
	using Info = Pylon::CDeviceInfo;
	using InfoList = Pylon::DeviceInfoList_t;

	InfoList filter;
	switch (ddt)
	{
		case DeviceDesignator::MAC:
		{
			filter.push_back(Info {}.SetMacAddress(designator));
			break;
		}
		case DeviceDesignator::SN:
		{
			filter.push_back(Info {}.SetSerialNumber(designator));
			break;
		}
		case DeviceDesignator::Unknown:
		{
			return nullptr;
			break;
		}
	}
	InfoList devices;
	tl_->EnumerateDevices(devices, filter);
	if (devices.size() != 1)
	{
		return nullptr;
	}
	return tl_->CreateDevice(devices.front());
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// ************************************************************************* //
