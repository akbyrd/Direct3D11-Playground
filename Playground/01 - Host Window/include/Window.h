#pragma once

class Window
{
protected:
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	virtual LRESULT MessageHandler(UINT, WPARAM, LPARAM) = 0;

	bool WinCreateWindow(
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
		HINSTANCE hInstance);

	HWND hwnd;
};