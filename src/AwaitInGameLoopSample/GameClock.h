#pragma once
#include <chrono>

// clock used to measure time between frames
class GameClock {
private:
	const std::chrono::steady_clock::time_point _startPoint;
	std::chrono::steady_clock::time_point _lastFramePoint;
	std::chrono::steady_clock::time_point _currentFramePoint;
	std::chrono::steady_clock::duration _lastFrameDuration;
public:
	GameClock() : _startPoint(std::chrono::steady_clock::now()), _lastFrameDuration(0) {
		_lastFramePoint = _currentFramePoint = _startPoint;

	}
	std::chrono::steady_clock::time_point startTime() const { return _startPoint; }
	std::chrono::steady_clock::time_point lastFrameTime() const { return _lastFramePoint; }
	std::chrono::steady_clock::time_point currentFrameTime() const { return _currentFramePoint; }
	std::chrono::steady_clock::duration lastFrameDuration() const { return _lastFrameDuration; }

	void onBeginNewFrame() {
		_lastFramePoint = _currentFramePoint;
		_currentFramePoint = std::chrono::steady_clock::now();
		_lastFrameDuration = _currentFramePoint - _lastFramePoint;
	}
};
