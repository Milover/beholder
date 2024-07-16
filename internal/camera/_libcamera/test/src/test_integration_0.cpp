/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>

#include "ConfigurationEventPrinter.h"
#include "ImageEventPrinter.h"

#include "ParamEntry.h"
#include "Params.h"
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

void dumpCameraParams(py::CInstantCamera& cam)
{
	camera::ParamList list {camera::getWritableParams(cam.GetNodeMap())};
	for (const auto& l : list)
	{
		std::cout << l.name << '\t' << l.value << '\n';
	}
}

const py::String_t CameraSN {"24491241"};
const py::String_t CameraMAC {"0030534487E9"};
const py::String_t CameraSubnetMask {"255.255.255.0"};
const py::String_t CameraGateway {"0.0.0.0"};
const py::String_t CameraIP {"192.168.1.85"};	// the new IP

} // end namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	using gigeTL = py::IGigETransportLayer;

	// Before using any pylon methods, the pylon runtime must be initialized.
	py::PylonAutoInitTerm autoInitTerm;

	try
	{
		// Setup transport layer
		py::CTlFactory& TlFactory {py::CTlFactory::GetInstance()};
		auto deleter = [&TlFactory](gigeTL* ptr) { TlFactory.ReleaseTl(ptr); };
		std::unique_ptr<gigeTL, decltype(deleter)> pTl
		(
			static_cast<gigeTL*>(TlFactory.CreateTl(py::BaslerGigEDeviceClass)),
			deleter
		);
		assert(pTl != nullptr);

		// Detect device(s)
		// set filters
		py::DeviceInfoList_t filter;
		filter.push_back(py::CDeviceInfo().SetMacAddress(camera::CameraMAC));

		// enumerate devices
		py::DeviceInfoList_t devices;
		pTl->EnumerateDevices(devices, filter);
		if (devices.empty())
		{
			std::cerr << "error: found no devices\n";
			return 1;
		}
		camera::printDevices(devices);

		// Setup device(s)
		// TODO: assert No. devices == No. provided MACs
		py::CInstantCamera cam {pTl->CreateDevice(devices.front())};

		// reset the config
		cam.RegisterConfiguration
		(
			static_cast<py::CConfigurationEventHandler*>(nullptr),
			py::RegistrationMode_ReplaceAll,
			py::Cleanup_None
		);
		// TODO: implement PTP: https://docs.baslerweb.com/precision-time-protocol

		/* NOTE: don't think we need events at this point
		// DONE: set up events, if necessary
		cam.RegisterConfiguration
		(
			new py::CConfigurationEventPrinter,
			py::RegistrationMode_Append,
			py::Cleanup_Delete
		);
		cam.RegisterImageEventHandler
		(
			new py::CImageEventPrinter,
			py::RegistrationMode_Append,
			py::Cleanup_Delete
		);
		cam.GrabCameraEvents = true;
		*/

		cam.Open();

		// disable all triggers, image compression and streaming
		// TODO: move this to a Configuration class
		py::CConfigurationHelper::DisableAllTriggers(cam.GetNodeMap());
		py::CConfigurationHelper::DisableCompression(cam.GetNodeMap());
		py::CConfigurationHelper::DisableGenDC(cam.GetNodeMap());

		/* NOTE: chunk data is mostly useless.
		 * Using functions provided by the CGrabResult.
		 * Move all of this to a Configuration class if we end up using it.
		// enable chunk features
		try
		{
			py::CBooleanParameter chunking {cam.GetNodeMap(), "ChunkModeActive"};
			chunking.SetValue(true);

			py::CEnumParameter selector {cam.GetNodeMap(), "ChunkSelector"};
			py::CBooleanParameter enable {cam.GetNodeMap(), "ChunkEnable"};
			auto activate = [&selector, &enable]
			(
				const py::String_t& avail,
				const py::String_t& name
			) -> bool
			{
				if (avail == name)
				{
					selector.SetValue(name);
					enable.SetValue(true);
					return true;
				}
				return false;
			};
			py::StringList_t available;
			selector.GetSettableValues(available);
			for (const auto& a : available)
			{
				// XXX: the availability of these changes from device to device
				// so we should not overly rely on these
				if (activate(a, "Timestamp")) { continue; }
				if (activate(a, "PayloadCRC16")) { continue; }
				if (activate(a, "FrameID")) { continue; }	// unavailable
				if (activate(a, "Framecounter")) { continue; }	// non-standard
			}
		}
		catch(py::GenericException& e)
		{
			std::cerr << "warning: "<< e.GetDescription() << '\n';
			// TODO: log/report that chunk features are disabled
		}
		*/

		// Set up runtime config
		// TODO: provide as function parameter
		camera::ParamList params;
		params.emplace_back("AcquisitionMode", "Continuous", camera::ParamType::Enum);
		params.emplace_back("TriggerSelector", "FrameStart", camera::ParamType::Enum);
		params.emplace_back("TriggerMode", "On", camera::ParamType::Enum);
		params.emplace_back("TriggerSource", "Line1", camera::ParamType::Enum);
		params.emplace_back("TriggerActivation", "RisingEdge", camera::ParamType::Enum);

		// apply config
		camera::setParams(cam.GetNodeMap(), params);
		//camera::dumpCameraParams(cam);

		// Acquire image(s)
		// XXX: should we provide our own buffers?
		// DONE: handle timeouts and/or data corruption
		// TODO: handle camera removal
		// 		XXX: may need to wait for ~1s within the catch
		// 		statement for the system to detect device removal
		// DONE: retrieve chunk data
		// TODO: communication between Go and C++
		// TODO: heartbeats?
		cam.StartGrabbing(1);
		py::CGrabResultPtr result;

		while (cam.IsGrabbing())
		{
			std::cout << "Waiting for image..." << '\n';
			bool success
			{
				cam.RetrieveResult(5000, result, py::TimeoutHandling_Return)
			};
			// NOTE: pass all timestamps to Go as a Unix time duration
			// in microseconds.
			const std::chrono::time_point<std::chrono::system_clock> timestamp
			{
				std::chrono::system_clock::now()
			};

			//if (success && result->GrabSucceeded())
			if (success)	// XXX: is this sufficient?
			{
				const uint8_t* img {static_cast<uint8_t*>(result->GetBuffer())};
				std::cout << "size x:       " << result->GetWidth() << '\n'
						  << "size y:       " << result->GetHeight() << '\n'
						  << "pixel value:  " << static_cast<uint32_t>(img[0]) << '\n'
						  << '\n'
						  << "image ID:     " << result->GetID() << '\n'
						  << "timestamp:    " << result->GetTimeStamp() << '\n'
						  << "has CRC:      " << result->HasCRC() << '\n'
						  << "CRC check:    " << result->CheckCRC() << '\n'
						  << "payload size: " << result->GetPayloadSize() << '\n'
						  << '\n';
				/* NOTE: chunk data is restrictive and impractical, so not using it.
				// get chunk data
				if (result->IsChunkDataAvailable())
				{
					camera::ParamList chunkParams
					{
						camera::getReadableParams(result->GetChunkDataNodeMap())
					};
					std::cout << "Framecounter: " << camera::getParameter("ChunkFramecounter", chunkParams).value << '\n'
							  << "FrameID:      " << camera::getParameter("ChunkFrameID", chunkParams).value << '\n'
							  << "Timestamp:    " << camera::getParameter("ChunkTimestamp", chunkParams).value << '\n'
							  << "CRC:          " << camera::getParameter("ChunkPayloadCRC16", chunkParams).value << '\n'
							  << '\n';
				}
				*/
			}
			else
			{
				std::cerr << "error: "
						  << std::hex << result->GetErrorCode()
						  << std::dec << " " << result->GetErrorDescription()
						  << '\n';
			}
		}

		// WARNING: docs say we should  call pTl->DestroyDevice(), so
		// we should check if this is ok, eg. with cam.HasOwnership()
		// or something similar
		cam.DestroyDevice();
	}
	catch(const py::GenericException& e)
	{
		// Error handling.
		std::cerr << "error: " << e.GetDescription() << '\n';
		return 1;
	}

	return 0;
}

// ************************************************************************* //
