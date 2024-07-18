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

#include <pylon/PylonIncludes.h>

#include "Camera.h"
#include "Exception.h"
#include "ParamEntry.h"
#include "TransportLayer.h"

#include "Test.h"

namespace py = Pylon;

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

void printDevices(const py::DeviceInfoList_t& devices)
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
		py::String_t activeMode;
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

const std::string CameraSN {"24491241"};
const std::string CameraMAC {"0030534487E9"};
const std::string CameraSubnetMask {"255.255.255.0"};
const std::string CameraGateway {"0.0.0.0"};
const std::string CameraIP {"192.168.1.85"};	// the new IP

const camera::ParamList CameraParameters
{
	{"AcquisitionMode",   "Continuous",   camera::ParamType::Enum},
	{"TriggerSelector",   "FrameStart",   camera::ParamType::Enum},
	{"TriggerMode",       "On",           camera::ParamType::Enum},
	{"TriggerSource",     "Line1",        camera::ParamType::Enum},
	{"TriggerActivation", "RisingEdge",   camera::ParamType::Enum},

	{"ChunkModeActive",   "true",         camera::ParamType::Bool},
	{"ChunkSelector",     "PayloadCRC16", camera::ParamType::Enum},
	{"ChunkEnable",       "true",         camera::ParamType::Bool}
};

} // end namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// Before using any pylon methods, the pylon runtime must be initialized.
	py::PylonAutoInitTerm autoInitTerm;

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

		cam.setParams(camera::CameraParameters);
		camera::dumpParams(cam.getParams());

		// Acquire image(s)
		cam.startAcquisition(1);
		py::CGrabResultPtr result;	// TODO: this guy

		while (cam.isAcquiring())
		{
			std::cout << "Waiting for image...\n";
			bool success {cam.acquire(result, std::chrono::seconds {2})};

			if (success && result->GrabSucceeded())
			{
				if (result->HasCRC() && !result->CheckCRC())
				{
					std::cerr << "acquisition error: CRC check failed\n";
					continue;
				}
				const uint8_t* img {static_cast<uint8_t*>(result->GetBuffer())};
				std::cout << "size x:          " << result->GetWidth() << '\n'
						  << "size y:          " << result->GetHeight() << '\n'
						  << "pixel value:     " << static_cast<uint32_t>(img[0]) << '\n'
						  << '\n'
						  << "image ID:        " << result->GetID() << '\n'
						  << "payload size:    " << result->GetPayloadSize() << '\n'
						  << '\n';
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
	}
	catch(const py::GenericException& e)
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
