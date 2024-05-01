/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	Common utility functions.

SourceFiles
	Preprocess.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_UTILITY_H
#define OCR_UTILITY_H

#include <cstring>
#include <memory>
#include <string>
#include <vector>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

//- Helper function for setting ch to raw.
//	If ch != nullptr, calls delete[] on ch.
void chPtrFromLiteral(char*& ch, const char* lit);

//- Convert a vector of strings into an array of char*.
std::unique_ptr<char*[]>
vecStr2ChPtrArr(const std::vector<std::string>& v);

//- Trim leading whitespace (left trim)
void trimWhiteL(std::string& s);

//- Trim trailing whitespace (right trim)
void trimWhiteR(std::string& s);

//- Trim leading and trailing whitespace (left-right trim)
void trimWhiteLR(std::string& s);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
