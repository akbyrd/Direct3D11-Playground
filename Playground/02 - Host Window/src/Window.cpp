#include "stdafx.h"

#include "Window.h"

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window *pThis = nullptr;

	//Retrieve the object pointer from the lParam and store it for future use
	if ( uMsg == WM_NCCREATE )
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = reinterpret_cast<Window*>(lpcs->lpCreateParams);

		pThis->hwnd = hwnd;
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
	}
	//Retrieve the stored object pointer
	else
	{
		LPARAM ptr = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
		pThis = reinterpret_cast<Window*>(ptr);
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
	case WM_NCDESTROY:
		//Clear stored pointer
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
		//TODO: Understand this
		//delete this;
		//Fall through

	default:
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
}

bool Window::WinCreateWindow(
	WNDCLASSEX wc,
	DWORD dwExStyle,
	LPCWSTR lpClassName,
	LPCWSTR lpWindowName,
	DWORD dwStyle,
	int X,
	int Y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance)
{
	//Register the window class
	RegisterClassExW(&wc);

	//Create the window with the screen settings and get the handle to it
	hwnd = CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle,
	                      X, Y, nWidth, nHeight,
	                      hWndParent, hMenu, hInstance, this);

	return hwnd != nullptr;
}