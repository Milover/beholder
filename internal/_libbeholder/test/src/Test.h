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

#include "Result.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * * Global Variables  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * * //

//- Wait for n seconds
template<typename Rep, typename Period>
void wait(const std::chrono::duration<Rep, Period>& duration)
{
	std::this_thread::sleep_for(duration);
}

//- Return a vector of T as a single string, by optionally quoting and appending
//	each T from the vector, using ',' as the separator.
template<typename T>
std::string inlineStrings(const std::vector<T> vec)
{
	std::stringstream ss;
	for (auto i {0ul}; i < vec.size(); ++i)
	{
		if constexpr (std::is_same_v<T, std::string>)
		{
			ss << std::quoted(vec[i]);
		}
		else
		{
			ss << vec[i];
		}
		if (i != vec.size() - 1)
		{
			ss << ", ";
		}
	}
	return ss.str();
}

//- Print a table row
template<typename... Ts>
void printTableRow(const std::vector<int>& widths, const Ts&... ss)
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

int checkOCRResults
(
	const std::vector<Result>& res,
	const std::vector<std::string>& expected
)
{
	std::vector<std::string> text;
	std::vector<double> conf;
	text.reserve(res.size());
	conf.reserve(res.size());
	for (const auto& r : res)
	{
		text.emplace_back(r.text);
		conf.emplace_back(r.confidence);
	}
	std::cout << "Expected output:  " << inlineStrings(expected) << '\n';
	std::cout << "OCR output:       " << inlineStrings(text) << '\n';
	std::cout << "OCR confidences:  " << inlineStrings(conf) << '\n';

	bool ok {text.size() == expected.size()};
	if (ok)
	{
		for (auto i {0ul}; i < text.size(); ++i)
		{
			ok = ok && text[i] == expected[i];
		}
	}
	else
	{
		std::cerr << "expected size: " << expected.size()
				  << ", but got size: " << text.size() << '\n';
	}
	return ok ? 0 : 1;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
