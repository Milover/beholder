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
#include <thread>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

void wait(int sec)
{
	const auto start {std::chrono::high_resolution_clock::now()};
	std::this_thread::sleep_for(std::chrono::seconds {sec});
}

// * * * * * * * * * * * * * Global Variables  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
