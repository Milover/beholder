/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A simple timer class

SourceFiles
	Timer.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_TIMER_H
#define OCR_TIMER_H

#include <chrono>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

/*---------------------------------------------------------------------------*\
                           Class Timer Declaration
\*---------------------------------------------------------------------------*/

class Timer
{
public:

	// Public typedefs

	using Clock = std::chrono::steady_clock;
	using TimePoint = std::chrono::time_point<Clock>;
	using Seconds = std::chrono::seconds;
	using Milliseconds = std::chrono::milliseconds;
	using Microseconds = std::chrono::microseconds;

private:

	// Private data

		TimePoint start_;
		TimePoint end_;
		bool running_ {false};


public:

	// Member functions

		//- Get duration from t1 to t2
		template<typename T = Milliseconds>
		double duration(const TimePoint& t1, const TimePoint& t2) const;

		//- Get duration from start to 'now' if running, or last interval
		template<typename T = Milliseconds>
		double elapsed() const;

		//- Get duration from tp to 'now'
		template<typename T = Milliseconds>
		double elapsed(const TimePoint& tp) const;

		//- Return running state
		bool isRunning() const;

		//- Return the current time point
		TimePoint now() const;

		//- Stop and reset the timer.
		void reset();

		//- Start/restart the timer and
		//	return the time point when it was started/restarted
		TimePoint start();

		//- Stop the timer and
		//	return the time point when it was stopped
		TimePoint stop();

};


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

template<typename T>
double Timer::duration
(
	const Timer::TimePoint& t1,
	const Timer::TimePoint& t2
) const
{
	return std::chrono::duration_cast<T>(t2 - t1).count();
}


template<typename T>
double Timer::elapsed(const Timer::TimePoint& tp) const
{
	return duration<T>(tp, now());
}


template<typename T>
double Timer::elapsed() const
{
	TimePoint endTime;

	if(running_)
		endTime = now();
	else
		endTime = end_;

	return duration<T>(start_, endTime);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
