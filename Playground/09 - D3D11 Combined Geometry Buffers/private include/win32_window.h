#pragma once

#include <Windows.h>
#include "Logging.h"
#include "Platform.h"

struct Window2
{
	InputQueue* inputQueue  = nullptr;
	HWND        hwnd        = nullptr;
	SIZE        minimumSize = {800, 600};
};

LRESULT CALLBACK
WndProc(Window2* window, HWND hwnd, uint32 uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		//TODO: Exit codes?
		//TODO: How should the application exit? Quit message? Inform the sim? Just queue input?
		case WM_CLOSE:
		{
			/* NOTE: Received when X button clicked, Alt-F4, SysMenu > Close clicked,
			* SysMenu double clicked, Right click Taskbar > Close
			*/
			PostQuitMessage(0);
			return 0;
		}

		case WM_ENDSESSION:
		{
			//NOTE: Application is shutting down, probably due to user logging off.
			//TODO: Perform an expedient shutdown here.
			return 0;
		}

		case WM_KEYDOWN:
		{
			if ( wParam == VK_ESCAPE )
				PostQuitMessage(0);
			return 0;
		}

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* minMaxInfo = (MINMAXINFO*) lParam;
			minMaxInfo->ptMinTrackSize.x = window->minimumSize.cx;
			minMaxInfo->ptMinTrackSize.y = window->minimumSize.cy;
			return 0;
		}
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK
StaticWndProc(HWND hwnd, uint32 uMsg, WPARAM wParam, LPARAM lParam)
{
	Window2 *window = nullptr;

	//NOTE: This misses the very first WM_GETMINMAXINFO.
	if (uMsg == WM_NCCREATE)
	{
		CREATESTRUCT *params = (CREATESTRUCT*) lParam;
		window = (Window2*) params->lpCreateParams;

		window->hwnd = hwnd;
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG) window);
	}
	else
	{
		window = (Window2*) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	}

	if (window)
		return WndProc(window, hwnd, uMsg, wParam, lParam);
	else
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

SIZE
ClientSizeToWindowSizeClamped(SIZE clientSize, SIZE maxSize, DWORD windowStyle)
{
	//TODO: Assert windowStyle != WS_OVERLAPPED. AdjustWindowRect docs say it's unsupported

	RECT windowRect = {0, 0, clientSize.cx, clientSize.cy};
	if ( !AdjustWindowRect(&windowRect, windowStyle, false) )
	{
		//TODO: Log
		//LOG_WARNING(L"Failed to translate client size to window size");
		return clientSize;
	}

	SIZE windowSize;
	windowSize.cx = windowRect.right - windowRect.left;
	windowSize.cy = windowRect.bottom - windowRect.top;

	//Clamp size
	if ( windowSize.cx > maxSize.cx )
		windowSize.cx = maxSize.cx;

	if ( windowSize.cy > maxSize.cy )
		windowSize.cy = maxSize.cy;

	return windowSize;
}

bool
InitializeWindow(Window2* window, InputQueue* inputQueue, HINSTANCE hInstance,
                 LPCWSTR applicationName, int32 nCmdShow, SIZE clientSize )
{
	window->inputQueue = inputQueue;

	WNDCLASS wc      = {};
	wc.style         = 0;
	wc.lpfnWndProc   = StaticWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(Window2*);
	wc.hInstance     = hInstance;
	wc.hIcon         = nullptr; //TODO: Legit icon
	wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = nullptr;
	wc.lpszClassName = applicationName;

	POINT windowPos  = {CW_USEDEFAULT, CW_USEDEFAULT};
	SIZE  windowSize = clientSize;

	DWORD windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_SIZEBOX
	                  | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
	                  | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	RECT usableDesktopRect = {};
	if ( SystemParametersInfoW(SPI_GETWORKAREA, 0, &usableDesktopRect, 0) )
	{
		SIZE usableDesktop = {
			usableDesktopRect.right - usableDesktopRect.left,
			usableDesktopRect.bottom - usableDesktopRect.top
		};

		window->minimumSize = ClientSizeToWindowSizeClamped(window->minimumSize, usableDesktop, windowStyle);
		windowSize  = ClientSizeToWindowSizeClamped(clientSize,  usableDesktop, windowStyle);

		windowPos = {
			(usableDesktop.cx - windowSize.cx) / 2,
			(usableDesktop.cy - windowSize.cy) / 2
		};
	}
	else
	{
		LOG_LASTERROR();
	}

	IF( RegisterClassW(&wc),
	   IS_FALSE, LOG_LASTERROR(); return false);

	//TODO: Log
	IF( window->hwnd = CreateWindowExW(0, applicationName, applicationName, windowStyle,
	   windowPos.x, windowPos.y, windowSize.cx, windowSize.cy,
	   nullptr, nullptr, hInstance, window),
	   IS_FALSE, LOG_LASTERROR(); return false);

	ShowWindow(window->hwnd, nCmdShow);

	return true;
}