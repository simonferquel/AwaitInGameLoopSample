#pragma once
#include "GameAwaitablePromise.h"
#include <chrono>
#include <DirectXMath.h>
#include "../DirectXTK/Src/AlignedNew.h"

// simple interpolation function
struct Interoplate_Linear {
	inline float operator()(float p) { return p; }
};

// generic linear interpolation
template<typename T>
T lerp(T start, T end, float progress) {
	return start + (end - start)*progress;
}

// matrix linear interpolation using DirectX Math SIMD enabled library
template <>
inline DirectX::XMFLOAT4X4A lerp(DirectX::XMFLOAT4X4A start, DirectX::XMFLOAT4X4A end, float progress) {
	DirectX::XMMATRIX xmStart = DirectX::XMLoadFloat4x4A(&start);
	DirectX::XMMATRIX xmEnd = DirectX::XMLoadFloat4x4A(&end);
	DirectX::XMMATRIX xmResult = xmStart + progress*(xmEnd - xmStart);
	DirectX::XMFLOAT4X4A result;
	DirectX::XMStoreFloat4x4A(&result, xmResult);
	return result;
}

__declspec(align(16))
struct aligned16{};

// animations are simple interpolations (with optionnaly an interpolation function), exposing an awaitable promise.
// this is a state machine based object exposed as an awaitable coroutine
template<typename T, typename TInterpolation = Interoplate_Linear>
class Animation : public aligned16, public DirectX::AlignedNew<Animation<T, TInterpolation>> {
private:
	T _startValue;
	T _endValue;
	GameAwaitableUniquePromise<void> _promise;
	std::chrono::steady_clock::duration _ellapsed;
	std::chrono::steady_clock::duration _duration;
	TInterpolation _interpolation;
public:
	Animation(const std::chrono::steady_clock::duration& duration, const T& startValue, const T& endValue, const TInterpolation& interpolation) :
		_ellapsed(0), _duration(duration), _startValue(startValue), _endValue(endValue), _interpolation(interpolation)
	{
	}
	T update(const std::chrono::steady_clock::duration& ellapsed, bool& ended) {
		if (_ellapsed >= _duration) {
			ended = true;
			return _endValue;
		}
		_ellapsed += ellapsed;
		if (_ellapsed >= _duration) {
			ended = true;
			// the animation just ended, raise the coroutine completion callback
			_promise.setResult();
		}
		else {
			ended = false;
		}
		float progress = ((float)_ellapsed.count()) / ((float)_duration.count());
		if (progress > 1) {
			progress = 1;
		}
		progress = _interpolation(progress);
		return lerp(_startValue, _endValue, progress);
	}

	GameAwaitableUniquePromise<void>& getPromise()  {
		return _promise;
	}
};
