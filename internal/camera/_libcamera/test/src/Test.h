/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	Global variables for the tests.

\*---------------------------------------------------------------------------*/

#ifndef CAMERA_TEST_H
#define CAMERA_TEST_H

#include <chrono>
#include <fstream>
#include <string>
#include <thread>

#include "ParamEntry.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

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

//- Write parameters to file
void writeParams(const std::string& filename, const ParamList& list)
{
	std::ofstream of {filename, std::ios::out | std::ios::trunc};
	for (const auto& l : list)
	{
		of << l.name << '\t' << l.value << '\n';
	}
}

// * * * * * * * * * * * * * Global Variables  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
