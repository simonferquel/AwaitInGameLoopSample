#include "stdafx.h"
#include "Timer.h"


Timer::Timer(const std::chrono::steady_clock::time_point& resumeTimePoint) : _resumeTimePoint(resumeTimePoint)
{
}

bool Timer::onTick(const std::chrono::steady_clock::time_point & currentTimePoint)
{
	if (currentTimePoint >= _resumeTimePoint) {
		_promise.setResult();
		return true;
	}
	return false;
}
