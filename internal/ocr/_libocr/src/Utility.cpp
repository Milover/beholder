/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include "Utility.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace ocr
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

void chPtrFromLiteral(char*& ch, const char* lit)
{
	if (ch)
	{
		delete[] (ch);
	}
	ch = new char[std::strlen(lit) + 1];
	std::strcpy(ch, lit);
}


std::unique_ptr<char*[]>
vecStr2ChPtrArr(const std::vector<std::string>& v)
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


void trimWhiteL(std::string& s)
{
	s.erase
	(
		s.begin(),
		std::find_if
		(
			s.begin(),
			s.end(),
			[](int ch) { return !std::isspace(ch); }
		)
	);
}


void trimWhiteR(std::string& s)
{
	s.erase
	(
		std::find_if
		(
			s.rbegin(),
			s.rend(),
			[](int ch) { return !std::isspace(ch); }
		).base(),
		s.end()
	);
}


void trimWhiteLR(std::string& s)
{
	trimWhiteL(s);
	trimWhiteR(s);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
