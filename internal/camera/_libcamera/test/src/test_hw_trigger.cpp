/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>

#include "Camera.h"
#include "Exception.h"
#include "Image.h"
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

	{"TriggerMode",                "On",                           camera::ParamType::Enum},
	{"TriggerSource",              "Line1",                        camera::ParamType::Enum},
	{"TriggerActivation",          "RisingEdge",                   camera::ParamType::Enum},

	{"LineSelector",               "Line2",                        camera::ParamType::Enum},
	{"LineMode",                   "Output",                       camera::ParamType::Enum},
	{"LineSource",                 "FrameTriggerWait",             camera::ParamType::Enum},
	{"LineInverter",               "true",                         camera::ParamType::Bool},

	{"ExposureMode",               "Timed",                        camera::ParamType::Enum},
	{"ExposureTimeAbs",            "1500",                         camera::ParamType::Int},

	{"ChunkModeActive",            "true",                         camera::ParamType::Bool},
	{"ChunkSelector",              "PayloadCRC16",                 camera::ParamType::Enum},
	{"ChunkEnable",                "true",                         camera::ParamType::Bool}
};

} // end namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// Before using any pylon methods, the pylon runtime must be initialized.
	Pylon::PylonAutoInitTerm autoInitTerm;

	try
	{
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
		//camera::dumpParams(cam.getParams(camera::ParamAccessMode::Read));

		// Acquire image(s)
		cam.startAcquisition(camera::CameraNImages);

		std::vector<std::unique_ptr<camera::Image>> images;
		images.reserve(camera::CameraNImages);

		while (cam.isAcquiring())
		{
			std::cout << "Acquiring...\n";
			try
			{
				std::unique_ptr<camera::Image> img
				{
					cam.acquire(std::chrono::seconds {2})
				};
				if (!img)
				{
					continue;
				}
				std::cout << "image id:   " << img->id << '\n'
						  << "image size: " << static_cast<double>(img->getRef().GetImageSize()) / 1000000.0 << "MB\n"
						  << '\n';
				images.emplace_back(img.release());
			}
			catch(const Pylon::TimeoutException& e)
			{
				std::cerr << "error: " << e.what() << '\n';
			}
			catch(const Pylon::GenericException& e)
			{
				if (!cam.isValid())
				{
					throw camera::Exception {"camera device removed", e};
				}
				throw e;
			}
		}
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
