/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A class for keeping track and analyzing timing data

SourceFiles
	TimeAnalyzer.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_TIMEANALYZER_H
#define OCR_TIMEANALYZER_H

#include <algorithm>
#include <optional>
#include <utility>
#include <type_traits>

#include "General.h"
#include "Timer.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

/*---------------------------------------------------------------------------*\
                     Class TimeAnalyzer Declaration
\*---------------------------------------------------------------------------*/

template<typename Key, typename U = std::enable_if_t<std::is_enum_v<Key>>>
class TimeAnalyzer
{
public:

	using Entry = std::pair<Key, Timer::TimePoint>;

private:

	template<typename T>
	inline static constexpr bool isKey_v = std::is_same_v<removeCVRef_t<T>, Key>;
	template<typename T>
	using enableIfKey_t = std::enable_if_t<isKey_v<T>>;

	// Private data

		std::vector<Entry> entries_;
		Timer t_;

public:

	// Member functions

		//- Get duration from time point at k1 to time point at k2.
		//  Returns 0 if k1 or k2 don't exist.
		template<typename T = Timer::Milliseconds>
		double duration(const Key& k1, const Key& k2) const;

		//- Get duration from the initial time point to time point at k.
		//  Returns 0 if k doesn't exist.
		template<typename T = Timer::Milliseconds>
		double duration(const Key& k) const;

		//- Get time point at k
		std::optional<Timer::TimePoint> at(const Key& k) const;

		//- Store the current time point with with the key k.
		//  Starts the timer if it is not running.
		template<typename T, typename = enableIfKey_t<T>>
		void mark(T&& t);

		//- Stop and reset the timer and clear all entries.
		void reset();

		//- Get a const reference to the timer
		const Timer& timer() const;

};


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

template<typename Key, typename U>
template<typename T>
double TimeAnalyzer<Key, U>::duration(const Key& k1, const Key& k2) const
{
	auto t1 {this->at(k1)};
	auto t2 {this->at(k2)};
	if (!t1 || !t2)
	{
		return 0.0;
	}
	return this->t_.template duration<T>(*t1, *t2);
}


template<typename Key, typename U>
template<typename T>
double TimeAnalyzer<Key, U>::duration(const Key& k) const
{
	return this->duration<T>(this->entries_.front().first, k);
}


template<typename Key, typename U>
std::optional<Timer::TimePoint> TimeAnalyzer<Key, U>::at(const Key& k) const
{
	auto found
	{
		std::find_if
		(
			this->entries_.begin(),
			this->entries_.end(),
			[&k](auto v) -> bool { return v.first == k; }
		)
	};
	if (found == this->entries_.end())
	{
		return std::nullopt;
	}
	return std::make_optional<Timer::TimePoint>(found->second);
}


template<typename Key, typename U>
template<typename T, typename>
void TimeAnalyzer<Key, U>::mark(T&& t)
{
	if (this->t_.isRunning())
	{
		this->entries_.emplace_back(std::forward<T>(t), this->t_.start());
	}
	else
	{
		this->entries_.emplace_back(std::forward<T>(t), this->t_.now());
	}
}


template<typename Key, typename U>
void TimeAnalyzer<Key, U>::reset()
{
	this->t_ = Timer {};
	this->entries_.clear();
}


template<typename Key, typename U>
const Timer& TimeAnalyzer<Key, U>::timer() const
{
	return this->t_;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
