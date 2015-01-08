#pragma once
#include <chrono>
#include "GameAwaitablePromise.h"

// timer state-machine exposed as an awaitable coroutine
class Timer
{
private:
	GameAwaitableUniquePromise<void> _promise;

	std::chrono::steady_clock::time_point _resumeTimePoint;
public:
	Timer(const std::chrono::steady_clock::time_point& resumeTimePoint);
	bool onTick(const std::chrono::steady_clock::time_point& currentTimePoint);
	GameAwaitableUniquePromise<void>& getPromise()  {
		return _promise;
	}
};

