#include "stdafx.h"

#include "Window.h"

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window *pThis = nullptr;

	//Store the object pointer packed in lParam for future use
	if ( uMsg == WM_NCCREATE )
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
		pThis = (Window*) lpcs->lpCreateParams;

		pThis->hwnd = hwnd;
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LPARAM) pThis);
	}
	//Retrieve the stored object pointer
	else
	{
		LPARAM ptr = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
		pThis = (Window*) ptr;
	}

	//Process the message
	if ( pThis )
		return pThis->MessageHandler(uMsg, wParam, lParam);
	else
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT Window::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( uMsg )
	{
		case WM_NCDESTROY: {
			//Clear stored pointer, then let Windows do its thing
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
			return DefWindowProcW(hwnd, uMsg, wParam, lParam);
		}

		default:
			return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
}

bool Window::WinCreateWindow(
	WNDCLASSEX wc,
	DWORD dwExStyle,
	LPCWSTR lpWindowName,
	DWORD dwStyle,
	uint16f X,
	uint16f Y,
	uint16f nWidth,
	uint16f nHeight,
	HMENU hMenu)
{
	//NOTE: Doesn't check for failures :(

	//For the window pointer we pack to get an instance reference
	wc.cbWndExtra += sizeof(Window*);

	//Register the window class
	RegisterClassExW(&wc);

	//Create the window with the screen settings and get the handle to it
	hwnd = CreateWindowExW(dwExStyle, wc.lpszClassName, lpWindowName, dwStyle,
	                       X, Y, nWidth, nHeight,
	                       nullptr, hMenu, wc.hInstance, this);

	return hwnd != nullptr;
}