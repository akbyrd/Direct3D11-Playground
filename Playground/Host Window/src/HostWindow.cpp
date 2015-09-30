#include "stdafx.h"
#include "HostWindow.h"
#include "ExitCode.h"

bool HostWindow::Initialize()
{
	//Get the instance of this application
	hInstance = GetModuleHandle(nullptr);

	//Give the application a name
	applicationName = L"Direct3D11 Playground";

	//Setup the windows class with default settings
	WNDCLASSEX wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(nullptr, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = nullptr;
	wc.lpszClassName = applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);

	//Determine the resolution of the desktop screen
	int desktopWidth  = GetSystemMetrics(SM_CXSCREEN);
	int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

	//Determine window size
	int windowWidth  = desktopWidth  > 800 ? 800 : desktopWidth;
	int windowHeight = desktopHeight > 600 ? 600 : desktopHeight;

	//Determine window position
	int windowPosX = (desktopWidth  - windowWidth ) / 2;
	int windowPosY = (desktopHeight - windowHeight) / 2;

	//Create the window with the screen settings and get the handle to it
	WinCreateWindow(wc, WS_EX_APPWINDOW, applicationName, applicationName,
	                WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
	                windowPosX, windowPosY, windowWidth, windowHeight,
	                nullptr, nullptr, hInstance);

	//Bring the window up on the screen and set it as main focus
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	gameTimer.Reset();
	gameTimer.Start();

	return true;
}

void HostWindow::Shutdown()
{
	//Show the mouse cursor
	ShowCursor(true);

	//Remove the window
	DestroyWindow(hwnd);
	hwnd = nullptr;

	//Remove the application instance
	UnregisterClass(applicationName, hInstance);
	hInstance = nullptr;
}

void HostWindow::Update()
{
	gameTimer.Tick();
	UpdateFrameStatistics();

	//Render
	//...

	return;
}

LRESULT HostWindow::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( uMsg )
	{
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
	case WM_KEYUP:
		if ( wParam == VK_ESCAPE )
			PostQuitMessage(0);
		return 0;

	//Send unhandled messages to the base class
	default:
		return __super::MessageHandler(uMsg, wParam, lParam);
	}
}

void HostWindow::UpdateFrameStatistics()
{
	const int bufferSize = 30;
	static double buffer[bufferSize];
	static int head = -1;
	static int length = 0;

	//Update the head position and length
	head = (head + 1) % bufferSize;
	if ( length < bufferSize - 1 )
		++length;

	//Update the head value
	buffer[head] = gameTimer.RealTime();

	int tail = (head - length) % bufferSize;
	if ( tail < 0 )
		tail += bufferSize;

	double delta = buffer[head] - buffer[tail];
	averageFrameTime = delta / length;

	return;
}