#pragma once
#include <Windows.h>
class dx_exception {
private:
	HRESULT _hr;
public:
	dx_exception(HRESULT hr) : _hr(hr) {}
	HRESULT getHR()const { return _hr; }
};

inline void throwIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw dx_exception(hr);
	}
}