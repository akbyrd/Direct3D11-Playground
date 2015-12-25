#include "stdafx.h"

#include "HostWindow.h"

bool   HostWindow::IsActive()      const { return isActive;      }
bool   HostWindow::IsMinimized()   const { return isMinimized;   }
bool   HostWindow::IsResizing()    const { return isResizing;    }
HWND   HostWindow::GetHWND()       const { return hwnd;          }
POINTS HostWindow::MousePosition() const { return mousePosition; }

bool HostWindow::Initialize(LPCWSTR applicationName, int iCmdshow,
                            int clientWidth, int clientHeight,
                            MessageQueue::Pusher* messageQueue)
{
	HostWindow::messageQueue = messageQueue;

	//Get the instance of this application
	hInstance = GetModuleHandleW(nullptr);

	//Give the application a name
	HostWindow::applicationName = applicationName;

	//Setup the windows class with default settings
	WNDCLASSEX wc = {};
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = nullptr;
	wc.lpszClassName = HostWindow::applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);

	DWORD windowStyle = WS_OVERLAPPEDWINDOW;

	//Determine the resolution of the desktop screen
	int desktopWidth  = GetSystemMetrics(SM_CXSCREEN);
	int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

	//Convert the min size to clamp the client area, not the whole window
	ClientSizeToWindowSize(minWidth, minHeight, desktopWidth, desktopHeight, windowStyle);

	//Convert the client size to a window size
	ClientSizeToWindowSize(clientWidth, clientHeight, desktopWidth, desktopHeight, windowStyle);

	//Center window
	int windowPosX = (desktopWidth  - clientWidth ) / 2;
	int windowPosY = (desktopHeight - clientHeight) / 2;

	//Create the window with the screen settings and get the handle to it
	WinCreateWindow(wc, WS_EX_APPWINDOW, applicationName, applicationName,
	                windowStyle,
	                windowPosX, windowPosY, clientWidth, clientHeight,
	                nullptr, nullptr, hInstance);

	//Bring the window up on the screen and set it as main focus
	ShowWindow(hwnd, iCmdshow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	return true;
}

bool HostWindow::ClientSizeToWindowSize(int &width, int &height,
                                        int desktopWidth, int desktopHeight, DWORD windowStyle)
{
	//Convert the min size to clamp the client area, not the whole window
	RECT windowRect = {};
	windowRect.left   = 0;
	windowRect.top    = 0;
	windowRect.right  = width;
	windowRect.bottom = height;

	BOOL ret = AdjustWindowRect(&windowRect, windowStyle, false);
	if ( !ret )
	{
		LOG_WARNING(L"Failed to translate client size to window size");
		return false;
	}

	width  = windowRect.right - windowRect.left;
	height = windowRect.bottom - windowRect.top;

	//Clamp window size to fit screen
	width  = std::min(width, desktopWidth);
	height = std::min(height, desktopHeight);

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
	UnregisterClassW(applicationName, hInstance);
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
		((MINMAXINFO*) lParam)->ptMinTrackSize.x = minWidth;
		((MINMAXINFO*) lParam)->ptMinTrackSize.y = minHeight;
		return 0;

	case WM_CLOSE:
		PostQuitMessage(1);
		return 0;

	case WM_DESTROY:
		messageQueue->PushMessage(Message::WindowClosed);
		return 0;

	case WM_MOUSEWHEEL:
	{
		//TODO: Should really check storing and incrementing the offset
		short delta = GET_WHEEL_DELTA_WPARAM(wParam);
		if ( delta > WHEEL_DELTA )
			messageQueue->PushMessage(Message::MouseWheelUp);
		else if ( delta < -WHEEL_DELTA )
			messageQueue->PushMessage(Message::MouseWheelDown);
		goto UpdateMousePosition;
	}

	case WM_RBUTTONDOWN:
		messageQueue->PushMessage(Message::MouseRightDown);
		goto UpdateMousePosition;

	case WM_RBUTTONUP:
		messageQueue->PushMessage(Message::MouseRightUp);
		goto UpdateMousePosition;

	case WM_LBUTTONDOWN:
		messageQueue->PushMessage(Message::MouseLeftDown);
		goto UpdateMousePosition;

	case WM_LBUTTONUP:
		messageQueue->PushMessage(Message::MouseLeftUp);
		goto UpdateMousePosition;

	UpdateMousePosition:
	case WM_MOUSEMOVE:
		mousePosition = MAKEPOINTS(lParam);
		return 0;

	//Send unhandled messages to the base class
	default:
		return __super::MessageHandler(uMsg, wParam, lParam);
	}
}