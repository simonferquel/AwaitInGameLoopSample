#include "stdafx.h"
#include "Engine.h"
#include <wrl.h>
#include <d3d11_2.h>
#include <dxgi1_3.h>
#include <DirectXColors.h>
#include <vector>
#include "Timer.h"
#include "AnimatedText.h"
#include "dx_exception.h"
using namespace std;
using namespace std::chrono;
using namespace Microsoft::WRL;





class Engine::impl {
private:
	HWND _hwnd;
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _ctx;
	ComPtr<IDXGISwapChain> _swapchain;
	ComPtr<ID3D11RenderTargetView> _rtv;
	D3D_FEATURE_LEVEL _featureLevel;
	GameClock _clock;
	DirectX::XMFLOAT4 _bgColor;
	vector<Timer> _activeTimers;
	vector<GameAwaitablePromise<void>> _clickAwaiters;
	vector<shared_ptr<SceneObject>> _sceneObjects;
public:
	impl(HWND hwnd) : _hwnd(hwnd), _bgColor(.0f, .0f, .0f, 1.0f) {
		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};
		RECT windowRect;
		GetWindowRect(hwnd, &windowRect);
		DXGI_SWAP_CHAIN_DESC scDesc;
		scDesc.BufferCount = 2;
		scDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scDesc.BufferDesc.Height = windowRect.bottom - windowRect.top;
		scDesc.BufferDesc.RefreshRate.Numerator = 60;
		scDesc.BufferDesc.RefreshRate.Denominator = 1;
		scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
		scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scDesc.BufferDesc.Width = windowRect.right - windowRect.left;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.Flags = 0;
		scDesc.OutputWindow = hwnd;
		scDesc.SampleDesc.Count = 1;
		scDesc.SampleDesc.Quality = 0;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scDesc.Windowed = true;
		UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		auto hr = D3D11CreateDeviceAndSwapChain(nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			flags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&scDesc,
			&_swapchain,
			&_device,
			&_featureLevel,
			&_ctx
			);
		throwIfFailed(hr);

		ComPtr<ID3D11Texture2D> buffer;
		throwIfFailed(_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), &buffer));
		throwIfFailed(_device->CreateRenderTargetView(buffer.Get(), nullptr, &_rtv));

		_ctx->OMSetRenderTargets(1, _rtv.GetAddressOf(), nullptr);
		_ctx->RSSetViewports(1, &CD3D11_VIEWPORT(.0, .0, (float)(windowRect.right - windowRect.left), (float)(windowRect.bottom - windowRect.top)));
	}
	void changeBackground(const DirectX::XMFLOAT4 & color) {
		_bgColor = color;
	}
	GameAwaitablePromise<void> waitFor(steady_clock::duration duration) {
		Timer result(_clock.currentFrameTime() + duration);
		_activeTimers.push_back(result);
		return result.getPromise();
	}
	void run() {
		_clock.onBeginNewFrame();
		for (int i = _activeTimers.size() - 1; i >= 0; --i) {
			if (_activeTimers[i].onTick(_clock.currentFrameTime())) {
				_activeTimers.erase(_activeTimers.begin() + i);
			}
		}
		for (auto& obj : _sceneObjects) {
			obj->updateState(_clock);
		}
		_ctx->ClearRenderTargetView(_rtv.Get(), (float*)&_bgColor);
		_ctx->OMSetRenderTargets(1, _rtv.GetAddressOf(), nullptr);
		UINT vpCount = 1;
		D3D11_VIEWPORT vp;
		_ctx->RSGetViewports(&vpCount, &vp);
		//_ctx->RSSetViewports(1, &CD3D11_VIEWPORT(.0, .0, (float)(windowRect.right - windowRect.left), (float)(windowRect.bottom - windowRect.top)));
		for (auto& obj : _sceneObjects) {
			obj->draw(_ctx.Get(), _rtv.Get());
		}
		_swapchain->Present(0, 0);
	}
	void onClick() {
		
		std::vector<GameAwaitablePromise<void>> toResume(std::move(_clickAwaiters));
		_clickAwaiters.clear();
		for (auto& i : toResume) {
			i.setResult();
		}
	}
	const GameClock& getClock() const {
		return _clock;
	}


	GameAwaitablePromise<void> waitForMouseClick() {
		GameAwaitablePromise<void> promise;
		_clickAwaiters.push_back(promise);
		return promise;
	}

	void addSceneObject(const std::shared_ptr<SceneObject>& object) {
		object->loadDeviceDependentResources(_device.Get());
		_sceneObjects.push_back(object);
	}
	void removeSceneObject(const std::shared_ptr<SceneObject>& object) {
		_sceneObjects.erase(std::find(_sceneObjects.begin(), _sceneObjects.end(), object));
	}
};

Engine::Engine(HWND hwnd,const std::function<void(Engine* engine)>& onStart) : _(std::make_unique<impl>(hwnd))
{

	onStart(this);
}


Engine::~Engine()
{
}

void Engine::run()
{
	_->run();
}

void Engine::onClick()
{
	_->onClick();
}

void Engine::changeBackground(const DirectX::XMFLOAT4 & color)
{
	_->changeBackground(color);
}

void Engine::addSceneObject(const std::shared_ptr<SceneObject>& object)
{
	_->addSceneObject(object);
}

void Engine::removeSceneObject(const std::shared_ptr<SceneObject>& object)
{
	_->removeSceneObject(object);
}

GameAwaitablePromise<void> Engine::waitFor(std::chrono::steady_clock::duration duration) {
	return _->waitFor(duration);
}


GameAwaitablePromise<void> Engine::waitForMouseClick() {
	return _->waitForMouseClick();
}
