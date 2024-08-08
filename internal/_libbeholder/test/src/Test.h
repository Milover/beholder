/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	Global variables and helper functions for the tests.

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_TEST_H
#define BEHOLDER_TEST_H

#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include <pylon/PylonIncludes.h>

#include "ParamEntry.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * * Global Variables  * * * * * * * * * * * * * * * //

//- Table widths for the device table
const std::vector<int> TableWidths {32, 14, 17, 17, 15, 8, 4, 6, 5};

// * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * * //

//- Wait for n seconds
template<typename Rep, typename Period>
void wait(const std::chrono::duration<Rep, Period>& duration)
{
	std::this_thread::sleep_for(duration);
}

//- Dump parameters to stdout
void dumpParams(const ParamList& list)
{
	for (const auto& l : list)
	{
		std::cout << l.name << '\t' << l.value << '\n';
	}
}

//- Return a vector of strings as a single string, by quoting and appending
//	each string from the vector, using ',' as the separator.
std::string inlineStrings(const std::vector<std::string> strs)
{
	std::stringstream ss;
	for (auto i {0ul}; i < strs.size(); ++i)
	{
		ss << std::quoted(strs[i]);
		if (i != strs.size() - 1)
		{
			ss << ", ";
		}
	}
	return ss.str();
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

//- Print a device table row
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

//- Print info for all devices in a table
void printDevices(const Pylon::DeviceInfoList_t& devices)
{
	beholder::printTableRows
	(
		beholder::TableWidths,
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
		beholder::printTableRows
		(
			beholder::TableWidths,
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
