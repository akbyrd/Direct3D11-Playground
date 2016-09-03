#pragma once

#include <Windows.h>
#include <windowsx.h>
#include "Platform.h"

/* Notes:
 * Functions bodies inside a type are implicitly inline.
 * Only function bodies marked inline or static are allowed in headers.
 * Static functions outside of a type are local to the translation unit.
 */

struct Win32State
{
	HWND      hwnd        = nullptr;
	SIZE      minimumSize = {200, 200};
	SimMemory simMemory   = {};
};

static inline LRESULT CALLBACK
WndProc(Win32State* win32State, HWND hwnd, u32 uMsg, WPARAM wParam, LPARAM lParam)
{
	InputState* input = &win32State->simMemory.input;

	switch (uMsg)
	{
		case WM_LBUTTONDOWN:
		{
			input->mouseLeft.isDown = true;
			input->mouseLeft.transitionCount++;
			goto UpdateMousePosition;
		}

		case WM_LBUTTONUP:
		{
			input->mouseLeft.isDown = false;
			input->mouseLeft.transitionCount++;
			goto UpdateMousePosition;
		}

		case WM_RBUTTONDOWN:
		{
			input->mouseRight.isDown = true;
			input->mouseRight.transitionCount++;
			goto UpdateMousePosition;
		}

		case WM_RBUTTONUP:
		{
			input->mouseRight.isDown = false;
			input->mouseRight.transitionCount++;
			goto UpdateMousePosition;
		}

		UpdateMousePosition:
		case WM_MOUSEMOVE:
		{
			input->mouseX = GET_X_LPARAM(lParam);
			input->mouseY = GET_Y_LPARAM(lParam);
			return 0;
		}

		case WM_KEYDOWN:
		{
			if ( wParam == VK_ESCAPE )
				PostQuitMessage(0);
			return 0;
		}

		case WM_MENUCHAR:
		{
			//Recieved unhandled keystroke while a menu is active.
			UINT menuType = HIWORD(wParam);
			bool isSystem = menuType == MF_SYSMENU;

			UINT keyPressed = LOWORD(wParam);
			bool enterPressed = keyPressed == VK_RETURN;

			//Don't beep when exiting fullscreen with Alt+Enter
			if (isSystem && enterPressed)
				return MAKELRESULT(0, MNC_CLOSE);

			break;
		}

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* minMaxInfo = (MINMAXINFO*) lParam;
			minMaxInfo->ptMinTrackSize.x = win32State->minimumSize.cx;
			minMaxInfo->ptMinTrackSize.y = win32State->minimumSize.cy;
			return 0;
		}

		//TODO: Exit codes?
		//TODO: How should the application exit? Quit message? Inform the sim? Just queue input?
		case WM_CLOSE:
		{
			/* NOTE: Received when the window's X button is clicked, Alt-F4,
			* SysMenu > Close clicked, SysMenu double clicked, Right click
			* Taskbar > Close, Task Manager Processes > End Task clicked
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
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK
StaticWndProc(HWND hwnd, u32 uMsg, WPARAM wParam, LPARAM lParam)
{
	Win32State* win32State = nullptr;

	//NOTE: This misses the very first WM_GETMINMAXINFO.
	if (uMsg == WM_NCCREATE)
	{
		CREATESTRUCT *params = (CREATESTRUCT*) lParam;
		win32State = (Win32State*) params->lpCreateParams;

		win32State->hwnd = hwnd;
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG) win32State);
	}
	else
	{
		win32State = (Win32State*) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	}

	if (win32State)
		return WndProc(win32State, hwnd, uMsg, wParam, lParam);
	else
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static inline SIZE
ClientSizeToWindowSizeClamped(SIZE clientSize, SIZE maxSize, DWORD windowStyle)
{
	//TODO: Assert windowStyle != WS_OVERLAPPED. AdjustWindowRect docs say it's unsupported

	RECT windowRect = {0, 0, clientSize.cx, clientSize.cy};
	if (!AdjustWindowRect(&windowRect, windowStyle, false))
	{
		//TODO: Log
		//LOG_WARNING(L"Failed to translate client size to window size");
		return clientSize;
	}

	SIZE windowSize;
	windowSize.cx = windowRect.right - windowRect.left;
	windowSize.cy = windowRect.bottom - windowRect.top;

	//Clamp size
	if (windowSize.cx > maxSize.cx)
		windowSize.cx = maxSize.cx;

	if (windowSize.cy > maxSize.cy)
		windowSize.cy = maxSize.cy;

	return windowSize;
}

static inline bool
InitializeWindow(Win32State* win32State, HINSTANCE hInstance, LPCWSTR applicationName, i32 nCmdShow, SIZE clientSize)
{
	WNDCLASS wc      = {};
	wc.style         = 0;
	wc.lpfnWndProc   = StaticWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(Win32State*);
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
	if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &usableDesktopRect, 0))
	{
		SIZE usableDesktop = {
			usableDesktopRect.right - usableDesktopRect.left,
			usableDesktopRect.bottom - usableDesktopRect.top
		};

		win32State->minimumSize = ClientSizeToWindowSizeClamped(win32State->minimumSize, usableDesktop, windowStyle);
		windowSize = ClientSizeToWindowSizeClamped(clientSize,  usableDesktop, windowStyle);

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

	IF( win32State->hwnd = CreateWindowExW(
			0,
			applicationName, applicationName,
			windowStyle,
			windowPos.x, windowPos.y,
			windowSize.cx, windowSize.cy,
			nullptr, nullptr,
			hInstance,
			win32State),
		IS_FALSE, LOG_LASTERROR(); return false);

	ShowWindow(win32State->hwnd, nCmdShow);

	return true;
}