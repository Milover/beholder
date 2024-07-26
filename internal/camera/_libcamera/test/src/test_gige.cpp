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

#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>

#include "Test.h"

namespace py = Pylon;

// * * * * * * * * * * * * * * * * Constants * * * * * * * * * * * * * * * * //

namespace camera
{


const std::string Usage = R"(
Usage: Utility_IpConfig <MAC> <IP> [MASK] [GATEWAY]
    <MAC> is the MAC address without separators, e.g., 0030531596CF
    <IP> is one of the following:
        - AUTO to use Auto-IP (LLA).
        - DHCP to use DHCP.
        - Everything else is interpreted as a new IP address in dotted notation, e.g., 192.168.1.1
    [MASK] is the network mask in dotted notation. This is optional. 255.255.255.0 is used as default.
    [GATEWAY] is the gateway address in dotted notation. This is optional. 0.0.0.0 is used as default.
Please note that this is a sample and no sanity checks are made.
)";

const std::vector<int> TableWidths {32, 14, 17, 17, 15, 8, 4, 6, 5};

void printTableHeader()
{
	std::cout << std::endl;
	std::cout.width( 32 + 14 + 17 + 17 + 15 + 8 );
	std::cout << "Available Devices";
	std::cout.width( 15 );
	std::cout << "   supports " << std::endl;
}

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

void handleFailure(const py::DeviceInfoList_t& devices)
{
	// Print usage information.
	std::cout << camera::Usage;
	camera::printTableHeader();
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

	// Print information table.
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


} // end namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// Before using any pylon methods, the pylon runtime must be initialized.
	py::PylonAutoInitTerm autoInitTerm;

	// set formatting
	std::cout << std::left << std::setfill(' ');

	try
	{
		// Create GigE transport layer.
		py::CTlFactory& TlFactory = py::CTlFactory::GetInstance();
		py::IGigETransportLayer* pTl = dynamic_cast<py::IGigETransportLayer*>(TlFactory.CreateTl(py::BaslerGigEDeviceClass));
		if (pTl == nullptr)
		{
			std::cerr << "Error: No GigE transport layer installed.\n"
					  << "Please install GigE support as it is required for this sample.";
			std::exit(EXIT_FAILURE);
		}

		// Enumerate devices.
		py::DeviceInfoList_t devices;
		pTl->EnumerateAllDevices(devices);

		// Check if enough parameters are given.
		if (argc < 3)
		{
			camera::handleFailure(devices);
			std::exit(EXIT_FAILURE);
		}

		// Read arguments. Note that sanity checks are skipped for clarity.
		py::String_t macAddress = argv[1];
		py::String_t ipAddress = argv[2];
		py::String_t subnetMask = "255.255.255.0";
		if (argc >= 4)
		{
			subnetMask = argv[3];
		}
		py::String_t defaultGateway = "0.0.0.0";
		if (argc >= 5)
		{
			defaultGateway = argv[4];
		}

		// Check if configuration mode is AUTO, DHCP, or IP address.
		bool isAuto = std::strcmp(argv[2], "AUTO") == 0;
		bool isDhcp = std::strcmp(argv[2], "DHCP") == 0;
		bool isStatic = !isAuto && !isDhcp;

		// Find the camera's user-defined name.
		py::String_t userDefinedName = "";
		for (const auto& d : devices)
		{
			if (macAddress == d.GetMacAddress())
			{
				userDefinedName = d.GetUserDefinedName();
			}
		}

		// Set new IP configuration.
		if
		(
			pTl->BroadcastIpConfiguration
			(
				macAddress,
				isStatic,
				isDhcp,
				ipAddress,
				subnetMask,
				defaultGateway,
				userDefinedName
			)
		)
		{
			pTl->RestartIpConfiguration(macAddress);
			std::cout << "Successfully changed IP configuration via broadcast for device "
					  << macAddress
					  << " to " << ipAddress << '\n';
		}
		else
		{
			std::cout << "Failed to change IP configuration via broadcast for device "
					  << macAddress << '\n'
					  << "This is not an error. The device may not support broadcast IP configuration.\n";
		}

		// Comment the following two lines to disable waiting on exit.
		std::cerr << '\n' << "Press enter to exit.\n";
		while (std::cin.get() != '\n');

		// Release transport layer.
		TlFactory.ReleaseTl( pTl );
	}
	catch (const py::GenericException& e)
	{
		// Error handling.
		std::cerr << "An exception occurred.\n"
		          << e.GetDescription() << '\n';
		std::exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

// ************************************************************************* //
