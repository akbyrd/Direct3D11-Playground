#pragma once

#include "Common.h"

class Window
{
protected:
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	virtual LRESULT MessageHandler(UINT, WPARAM, LPARAM) = 0;

	bool WinCreateWindow(
		WNDCLASSEX wc,
		DWORD dwExStyle,
		LPCWSTR lpWindowName,
		DWORD dwStyle,
		uint16f X,
		uint16f Y,
		uint16f nWidth,
		uint16f nHeight,
		HMENU hMenu);

	HWND hwnd;
};