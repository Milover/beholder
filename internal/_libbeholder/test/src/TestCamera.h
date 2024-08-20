/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	Global variables and helper functions for the tests.

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_TEST_CAMERA_H
#define BEHOLDER_TEST_CAMERA_H

#include <iomanip>
#include <fstream>
#include <string>
#include <vector>

#include <pylon/PylonIncludes.h>

#include "ParamEntry.h"
#include "Test.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * * Global Variables  * * * * * * * * * * * * * * * //

//- Table widths for the device table
const std::vector<int> CameraTableWidths {32, 14, 17, 17, 15, 8, 4, 6, 5};

// * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * * //

//- Dump parameters to stdout
void dumpParams(const ParamList& list)
{
	for (const auto& l : list)
	{
		std::cout << l.name << '\t' << l.value << '\n';
	}
}

//- Write parameters to file
void writeParams(const std::string& filename, const ParamList& list)
{
	std::ofstream of {filename, std::ios::out | std::ios::trunc};
	for (const auto& l : list)
	{
		of << l.name << '\t' << l.value << '\n';
	}
}

//- Print info for all devices in a table
void printDevices(const Pylon::DeviceInfoList_t& devices)
{
	beholder::printTableRow
	(
		beholder::CameraTableWidths,
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
		beholder::printTableRow
		(
			beholder::CameraTableWidths,
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

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
