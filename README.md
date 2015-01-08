AwaitInGameLoopSample
=====================

This sample demonstrate a not so obvious use of await and awaitable coroutines for video games or other rendering loop based programs.
Await is not tied to any type of coroutines (it is not dedicated to thread and async IO) and it has been designed to be extensible : any developper can wrap any kind of coroutine in an awaitable aware object, and that is great !

This sample is a Direct3D based application that runs entirely within 1 thread (the render loop).

In this sample, I created a simple and lightweight promise class that has an await-compatible interface, and I used it to expose different kind of coroutine:
- Timer : is a simple time tracking state machine (updated at each frame). it exposes its completion as a promise.
- MouseClick : the engine exposes user input as awaitable coroutines as well
- Animations : are simple state machines that update a value (a transformation matrix, an opacity) until completion. Animation completion are also exposed as promises

The game logic rely on animation timings, timers and user input (it runs a fade in animation to show a message, waits for a click, runs a fadeout animation, change the background color, wait for 0.5 seconds, and restart in a loop) and traditionnaly would have been implemented in a quite complex state machine (updated at each frame and on user input). With awaitable coroutines, this logic that is conceptually a simple loop, can be expressed exactly like that, without blocking the UI and without needing an additional thread than the render loop :
```C++
// the whole state machine is expressed in a simple and readable sequencial way
concurrency::task<void> gameLogic(Engine* engine) {
	auto animatedText = std::make_shared<AnimatedText>();
	engine->addSceneObject(animatedText);
	std::default_random_engine re;
	std::uniform_real_distribution<float> dist(0.0f, 0.7f);
	while (true) {
		__await animatedText->fadeIn();
		__await engine->waitForMouseClick();

		__await animatedText->fadeOut();
		engine->changeBackground(DirectX::XMFLOAT4(dist(re), dist(re), dist(re), 1.0f));

		__await engine->waitFor(duration_cast<steady_clock::duration>(.5s));
	}

}
```
