#pragma once
#include <Windows.h>
#include <memory>
#include <DirectXMath.h>
#include "GameAwaitablePromise.h"
#include "SceneObject.h"
#include <chrono>
#include <functional>

// main engine
// handles the main game loop (run function is called in the windows message loop on idle)
class Engine
{
private:
	class impl;
	std::unique_ptr<impl> _;
public:
	Engine(HWND hwnd, const std::function<void(Engine* engine)>& onStart);
	~Engine();
	void run();
	void onClick();
	void changeBackground(const DirectX::XMFLOAT4& color);
	void addSceneObject(const std::shared_ptr<SceneObject>& object);
	void removeSceneObject(const std::shared_ptr<SceneObject>& object);
	// the timers are implemented as simple state machines (updated at each run call) 
	// and exposed as awaitable coroutines
	GameAwaitableUniquePromise<void>& waitFor(std::chrono::steady_clock::duration duration);
	// user input can also be exposed as an awaitable coroutine
	GameAwaitableUniquePromise<void>& waitForMouseClick();
};


