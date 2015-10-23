#include "stdafx.h"
#include "HostWindow.h"

bool HostWindow::IsActive()    const { return isActive;    }
bool HostWindow::IsMinimized() const { return isMinimized; }
bool HostWindow::IsResizing()  const { return isResizing;  }
HWND HostWindow::GetHWND()     const { return hwnd;        }

bool HostWindow::Initialize(LPCWSTR applicationName, int iCmdshow,
                            int clientWidth, int clientHeight,
                            MessageQueue::Pusher* messageQueue)
{
	HostWindow::messageQueue = messageQueue;

	//Get the instance of this application
	hInstance = GetModuleHandle(nullptr);

	//Give the application a name
	HostWindow::applicationName = applicationName;

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

	DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	//Determine the resolution of the desktop screen
	int desktopWidth  = GetSystemMetrics(SM_CXSCREEN);
	int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

	//Convert the client size to a window size
	RECT windowRect;
	windowRect.left   = 0;
	windowRect.top    = 0;
	windowRect.right  = clientWidth;
	windowRect.bottom = clientHeight;

	int windowWidth;
	int windowHeight;

	if ( AdjustWindowRect(&windowRect, windowStyle, false) )
	{
		windowWidth  = windowRect.right - windowRect.left;
		windowHeight = windowRect.bottom - windowRect.top;
	}
	else
	{
		windowWidth  = clientWidth;
		windowHeight = clientHeight;

		DWORD ret = GetLastError();
		std::wstring warning = L"WARNING: Failed to translate client size to window size";
		warning += std::to_wstring(ret);
		OutputDebugString(warning.c_str());
		__debugbreak();
	}

	//Clamp window size to fit screen
	windowWidth  = std::min(windowWidth,  desktopWidth);
	windowHeight = std::min(windowHeight, desktopHeight);

	//Center window
	int windowPosX = (desktopWidth  - windowWidth ) / 2;
	int windowPosY = (desktopHeight - windowHeight) / 2;

	//Create the window with the screen settings and get the handle to it
	WinCreateWindow(wc, WS_EX_APPWINDOW, applicationName, applicationName,
	                windowStyle,
	                windowPosX, windowPosY, windowWidth, windowHeight,
	                nullptr, nullptr, hInstance);

	//Bring the window up on the screen and set it as main focus
	ShowWindow(hwnd, iCmdshow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	return true;
}

void HostWindow::Teardown()
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

LRESULT HostWindow::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( uMsg )
	{
	//TODO: Move to input in game
	case WM_KEYDOWN:
	case WM_KEYUP:
		if ( wParam == VK_ESCAPE )
			PostQuitMessage(0);
		return 0;

	case WM_ACTIVATE:
		if ( LOWORD(wParam) == WA_INACTIVE )
		{
			isActive = false;
			messageQueue->PushMessage(Message::WindowInactive);
		}
		else
		{
			isActive = true;
			messageQueue->PushMessage(Message::WindowActive);
		}
		return 0;

	//Includes moves
	case WM_ENTERSIZEMOVE:
		isResizing = true;
		messageQueue->PushMessage(Message::WindowResizingBegin);
		return 0;

	//Includes moves
	case WM_EXITSIZEMOVE:
		isResizing = false;
		messageQueue->PushMessage(Message::WindowResizingEnd);
		messageQueue->PushMessage(Message::WindowSizeChanged);
		return 0;

	case WM_SIZE:
		switch ( wParam )
		{
		case SIZE_MINIMIZED:
			isMinimized = true;
			messageQueue->PushMessage(Message::WindowMinimized);
			break;

		case SIZE_MAXIMIZED:
			isMaximized = true;
			messageQueue->PushMessage(Message::WindowSizeChanged);
			break;

		/* Sent for any size event that isn't covered above, doesn't mean an
		 * actual 'restore' operation on the window.
		*/
		case SIZE_RESTORED:
			//Actual restore operation
			if ( isMinimized )
			{
				isMinimized = false;
				messageQueue->PushMessage(Message::WindowUnminimized);
			}
			//Actual restore operation
			else if ( isMaximized )
			{
				isMaximized = false;
				messageQueue->PushMessage(Message::WindowSizeChanged);
			}
			//Just a resize operation
			else if ( !isResizing )
			{
				messageQueue->PushMessage(Message::WindowSizeChanged);
			}
			break;
		}
		return 0;

	case WM_GETMINMAXINFO:
		//Limit the minimum window size
		((MINMAXINFO*) lParam)->ptMinTrackSize.x = 640;
		((MINMAXINFO*) lParam)->ptMinTrackSize.y = 480;
		return 0;

	case WM_CLOSE:
		PostQuitMessage(1);
		return 0;

	case WM_DESTROY:
		messageQueue->PushMessage(Message::WindowClosed);
		return 0;

	//Send unhandled messages to the base class
	default:
		return __super::MessageHandler(uMsg, wParam, lParam);
	}
}