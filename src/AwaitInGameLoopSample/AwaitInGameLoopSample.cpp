// AwaitInGameLoopSample.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "AwaitInGameLoopSample.h"
#include "Engine.h"
#include <future>
#include <memory>
#include <random>
#include <ppltasks.h>
#include "AnimatedText.h"
#include <chrono>
#include "GameAwaitablePromise.h"
#include <pplawait.h>


using namespace std::chrono;

// the whole state machine is expressed in a simple and readable sequencial way
GameAwaitableSharedPromise<void> gameLogic(Engine* engine) {
	auto animatedText = std::make_shared<AnimatedText>();
	engine->addSceneObject(animatedText);
	std::default_random_engine re((unsigned int)(std::chrono::steady_clock::now().time_since_epoch().count()));
	std::uniform_real_distribution<float> dist(0.0f, 0.7f);
	while (true) {
		__await animatedText->fadeIn();
		__await engine->waitForMouseClick();

		__await animatedText->fadeOut();
		engine->changeBackground(DirectX::XMFLOAT4(dist(re), dist(re), dist(re), 1.0f));

		__await engine->waitFor(duration_cast<steady_clock::duration>(.5s));
	}

}



#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
HWND				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	/* normal win32 initialization code */
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	MyRegisterClass(hInstance);

	HWND window = InitInstance(hInstance, nCmdShow);
	// Perform application initialization:
	if (!window)
	{
		return FALSE;
	}

	// instanciation of the engine (and registering the gameLogic function as the startup callback)
	Engine engine(window, [](Engine* e) {
		gameLogic(e);
	});
	// Main message loop (if there is a message to process, process it, else, make the engine run):
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				break;
			}
			else if (msg.message == WM_LBUTTONDOWN) {
				engine.onClick();
			}
		}
		else {
			// game loop
			engine.run();
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AWAITINGAMELOOPSAMPLE));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"Sample Await in Game loop";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(L"Sample Await in Game loop", L"Sample Await in Game loop", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return hWnd;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
