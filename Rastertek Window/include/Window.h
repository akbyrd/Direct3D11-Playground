#include "stdafx.h"

class Window final
{
public:
	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	LPCWSTR   applicationName = nullptr;
	HINSTANCE hinstance       = nullptr;
	HWND      hwnd            = nullptr;
	bool      exit            = false;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static Window* ApplicationHandle = 0;