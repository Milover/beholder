/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include "Timer.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool Timer::isRunning() const
{
	return running_;
}


Timer::TimePoint Timer::now() const
{
	return Clock::now();
}


void Timer::reset()
{
	start_ = TimePoint {};
	end_ = TimePoint {};
	running_ = false;;
}


Timer::TimePoint Timer::start()
{
	start_ = now();
	running_ = true;

	return start_;
}


Timer::TimePoint Timer::stop()
{
	end_ = now();
	running_ = false;

	return end_;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
