/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	Common utility functions.

SourceFiles
	Utility.cpp

\*---------------------------------------------------------------------------*/

#ifndef CAMERA_UTILITY_H
#define CAMERA_UTILITY_H

#include <memory>
#include <string>
#include <vector>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

//- Helper function for setting ch to raw.
//	If ch != nullptr, calls delete[] on ch.
void chPtrFromLiteral(char*& ch, const char* lit);

//- Convert a vector of strings into an array of char*.
std::unique_ptr<char*[]>
vecStr2ChPtrArr(const std::vector<std::string>& v);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
