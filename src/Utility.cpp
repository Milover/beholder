/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <memory>
#include <string>
#include <vector>

#include "Utility.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace ocr
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

// Convert a vector of strings into an array of char*.
std::unique_ptr<char*[]>
vectorStrings2UniqueCharPtr(const std::vector<std::string>& v)
{
	std::unique_ptr<char*[]> result {new char*[v.size() + 1]};

	for (auto i {0ul}; i < v.size(); ++i)
	{
		result[i] = new char[v[i].length() + 1];
		std::strcpy(result[i], v[i].c_str());
	}
	result[v.size()] = nullptr;

	return result;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
