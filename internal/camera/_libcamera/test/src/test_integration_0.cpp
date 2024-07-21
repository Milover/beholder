/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <algorithm>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>

#include "Camera.h"
#include "Exception.h"
#include "ParamEntry.h"
#include "TransportLayer.h"

#include "Test.h"

// * * * * * * * * * * * * * * * * Constants * * * * * * * * * * * * * * * * //

namespace camera
{

const std::vector<int> TableWidths {32, 14, 17, 17, 15, 8, 4, 6, 5};

template<typename... Ts>
void printTableRows(const std::vector<int>& widths, const Ts&... ss)
{
	assert(widths.size() == sizeof...(ss));

	auto count {0ul};
	auto f = [&count, &widths] (const auto& s)
	{
		std::cout.width(widths[count]);
		std::cout << s;
		++count;
	};
	(f(ss), ...);

	std::cout << '\n';
}

void printDevices(const Pylon::DeviceInfoList_t& devices)
{
	camera::printTableRows
	(
		camera::TableWidths,
		"Friendly Name",
		"MAC",
		"IP Address",
		"Subnet Mask",
		"Gateway",
		"Mode",
		"IP?",
		"DHCP?",
		"LLA?"
	);

	for (const auto& d : devices)
	{
		// Determine current configuration mode.
		Pylon::String_t activeMode;
		if (d.IsPersistentIpActive())
		{
			activeMode = "Static";
		}
		else if (d.IsDhcpActive())
		{
			activeMode = "DHCP";
		}
		else
		{
			activeMode = "AutoIP";
		}
		camera::printTableRows
		(
			camera::TableWidths,
			d.GetFriendlyName(),
			d.GetMacAddress(),
			d.GetIpAddress(),
			d.GetSubnetMask(),
			d.GetDefaultGateway(),
			activeMode,
			d.IsPersistentIpSupported(),
			d.IsDhcpSupported(),
			d.IsAutoIpSupported()
		);
	}
}

void dumpParams(const ParamList& list)
{
	for (const auto& l : list)
	{
		std::cout << l.name << '\t' << l.value << '\n';
	}
}

// acA2440-20gm
//const std::string CameraSN {"24491241"};
//const std::string CameraMAC {"0030534487E9"};

// acA4024-8gc
const std::string CameraSN {"23096460"};
const std::string CameraMAC {"0030532F3F8C"};

const std::string CameraSubnetMask {"255.255.255.0"};
const std::string CameraGateway {"0.0.0.0"};
const std::string CameraIP {"192.168.1.85"};	// the new IP

const std::size_t CameraNImages {10};

const camera::ParamList CameraParameters
{
	{"AcquisitionMode",            "Continuous",                   camera::ParamType::Enum},
	{"TriggerSelector",            "FrameStart",                   camera::ParamType::Enum},

	{"TriggerMode",                "On",                           camera::ParamType::Enum},
	{"TriggerSource",              "Software",                     camera::ParamType::Enum},

//	{"TriggerMode",                "On",                           camera::ParamType::Enum},
//	{"TriggerSource",              "Line1",                        camera::ParamType::Enum},
//	{"TriggerActivation",          "RisingEdge",                   camera::ParamType::Enum},

//	{"ExposureMode",               "Timed",                        camera::ParamType::Enum},
//	{"ExposureTimeRaw",            "500",                          camera::ParamType::Int},

	//{"ChunkModeActive",            "true",                         camera::ParamType::Bool},
	//{"ChunkSelector",              "PayloadCRC16",                 camera::ParamType::Enum},
	//{"ChunkEnable",                "true",                         camera::ParamType::Bool}
};

} // end namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// Before using any pylon methods, the pylon runtime must be initialized.
	Pylon::PylonAutoInitTerm autoInitTerm;

	try
	{
		/* TODO: add to TransportLayer
		Pylon::CTlFactory& factory {Pylon::CTlFactory::GetInstance()};
		Pylon::IGigETransportLayer* tl_ {static_cast<Pylon::IGigETransportLayer*>(factory.CreateTl(Pylon::BaslerGigEDeviceClass))};

		//Pylon::DeviceInfoList_t filter;
		//filter.push_back(Pylon::CDeviceInfo {}.SetMacAddress(camera::CameraMAC.c_str()));
		//filter.push_back(Pylon::CDeviceInfo {}.SetSerialNumber(camera::CameraSN.c_str()));
		Pylon::DeviceInfoList_t devices;
		tl_->EnumerateAllDevices(devices);		// XXX: not the same as EnumerateDevices!!!
		camera::printDevices(devices);

		tl_->BroadcastIpConfiguration
		(
			camera::CameraMAC.c_str(),
			false,
			true,
			camera::CameraIP.c_str(),
			camera::CameraSubnetMask.c_str(),
			camera::CameraGateway.c_str(),
			devices[0].GetUserDefinedName()
		);
		tl_->RestartIpConfiguration(camera::CameraMAC.c_str());
		Pylon::CTlFactory::GetInstance().ReleaseTl(tl_);
		*/
		// Create transport layer
		camera::TransportLayer tl {camera::DeviceClass::GigE};

		// Create camera device and apply configuration
		camera::Camera cam
		{
			tl.createDevice(camera::CameraMAC, camera::DeviceDesignator::MAC)
		};
		assert(cam.isValid());

		if (!cam.setParams(camera::CameraParameters))
		{
			std::cerr << "warning: some parameters not set\n";
		}
		camera::dumpParams(cam.getParams());
		//camera::dumpParams(cam.getParams(camera::ParamAccessMode::Read));

		// Acquire image(s)
		cam.startAcquisition(camera::CameraNImages);
		std::vector<Pylon::CPylonImage> images;
		images.reserve(camera::CameraNImages);

		while (cam.isAcquiring())
		{
			std::cout << "Waiting for image...\n";

			Pylon::CGrabResultPtr result;	// TODO: this guy
			bool success {cam.acquire(result, std::chrono::seconds {2})};
			if (success && result->GrabSucceeded())
			{
				if (result->HasCRC() && !result->CheckCRC())
				{
					std::cerr << "acquisition error: CRC check failed\n";
					continue;
				}
				const uint8_t* img {static_cast<uint8_t*>(result->GetBuffer())};
				std::cout << "image id:     " << result->GetID() << '\n'
						  << "payload size: " << static_cast<double>(result->GetPayloadSize()) / 1000000.0 << "MB\n"
						  << "pixel value:  " << static_cast<uint32_t>(img[0]) << '\n'
						  << '\n';

//				Pylon::CPylonImage image;
//				image.AttachGrabResultBuffer(result);
//				result.Release();
			}
			else if (success)
			{
				std::cerr << "acquisition error: "
						  << std::hex << result->GetErrorCode()
						  << std::dec << " " << result->GetErrorDescription()
						  << '\n';
			}
			else
			{
				if (!cam.isValid())
				{
					throw camera::Exception {"camera device removed"};
				}
				std::cerr << "acquisition error: timed-out\n";
			}
		}

		// Write image(s) to file(s)
//		auto imgCount {0ul};
//		for (auto& i : images)
//		{
//			i.Save
//			(
//				Pylon::ImageFileFormat_Png,
//				(std::string{"image_"} + std::to_string(imgCount) + std::string{".png"}).c_str()
//			);
//			i.Release();
//			++imgCount;
//		}
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "error: " << e.what() << '\n';
		return 1;
	}
	catch(const camera::Exception& e)
	{
		std::cerr << "error: " << e.what() << '\n';
		return 1;
	}
	catch(...)
	{
		std::cerr << "error: unknown\n";
		return 1;
	}

	return 0;
}

// ************************************************************************* //
