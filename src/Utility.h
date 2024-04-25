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

#include <memory>
#include <string>
#include <vector>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

// Convert a vector of strings into an array of char*.
std::unique_ptr<char*[]>
vectorStrings2UniqueCharPtr(const std::vector<std::string>& v);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
