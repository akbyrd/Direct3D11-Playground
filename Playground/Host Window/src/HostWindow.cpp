#include "stdafx.h"
#include "HostWindow.h"
#include "ExitCode.h"

bool HostWindow::IsFocused() const { return isActive && !isMinimized; }
HWND HostWindow::GetHWND() const { return hwnd; }

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
	windowWidth  = std::min(windowWidth, desktopWidth);
	windowHeight = std::min(windowHeight, desktopHeight);

	//Determine window position
	int windowPosX = (desktopWidth  - windowWidth ) / 2;
	int windowPosY = (desktopHeight - windowHeight) / 2;

	//Create the window with the screen settings and get the handle to it
	WinCreateWindow(wc, WS_EX_APPWINDOW, applicationName, applicationName,
	                WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
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

long HostWindow::Teardown()
{
	gameTimer.Stop();

	//Show the mouse cursor
	ShowCursor(true);

	//Remove the window
	DestroyWindow(hwnd);
	hwnd = nullptr;

	//Remove the application instance
	UnregisterClass(applicationName, hInstance);
	hInstance = nullptr;

	return ExitCode::Success;
}

long HostWindow::Update()
{
	gameTimer.Tick();
	UpdateFrameStatistics();

	if ( isResizing ) { return ExitCode::Success; }

	//Render
	//...

	return ExitCode::Success;
}

long HostWindow::Resize()
{
	return ExitCode::Success;
}

LRESULT HostWindow::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( uMsg )
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
		if ( wParam == VK_ESCAPE )
			PostQuitMessage(0);
		return 0;

	//TODO: This should probably cap the frame rate of the application
	//Pause the game timer when the window loses focus.
	case WM_ACTIVATE:
		if ( LOWORD(wParam) == WA_INACTIVE )
		{
			gameTimer.Stop();
			isActive = false;
		}
		else
		{
			gameTimer.Start();
			isActive = true;
		}
		return 0;

	//Includes moves
	case WM_ENTERSIZEMOVE:
		gameTimer.Stop();
		isResizing = true;
		return 0;

	//Includes moves
	case WM_EXITSIZEMOVE:
		gameTimer.Start();
		isResizing = false;
		Resize();
		return 0;

	case WM_SIZE:
		windowWidth  = LOWORD(lParam);
		windowHeight = HIWORD(lParam);
		switch ( wParam )
		{
		case SIZE_MINIMIZED:
			gameTimer.Stop();
			isMinimized = true;
			break;

		case SIZE_MAXIMIZED:
			isMaximized = true;
			break;

		/* Sent for any size event that isn't covered above, doesn't mean an
		 * actual 'restore' operation on the window.
		*/
		case SIZE_RESTORED:
			//Actual restore operation
			if ( isMinimized )
			{
				gameTimer.Start();
				isMinimized = false;
				Resize();
			}
			else if ( isMaximized )
			{
				isMaximized = false;
				Resize();
			}
			else if ( !isResizing )
			{
				Resize();
			}
			break;
		}
		return 0;

	case WM_GETMINMAXINFO:
		//Limit the minimum window size
		((MINMAXINFO*) lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*) lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_CLOSE:
	case WM_DESTROY:
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
	static double deltaToMS;

	//Update the head position and length
	head = (head + 1) % bufferSize;
	if ( length < bufferSize - 1 )
	{
		++length;
		deltaToMS = 1000. / length;
	}

	//Update the head value
	buffer[head] = gameTimer.RealTime();

	int tail = (head - length) % bufferSize;
	if ( tail < 0 )
		tail += bufferSize;

	double delta = buffer[head] - buffer[tail];
	averageFrameTime = delta * deltaToMS;

	return;
}