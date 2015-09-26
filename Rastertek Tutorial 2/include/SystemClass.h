#include "stdafx.h"
#include "InputClass.h"
#include "GraphicsClass.h"

class SystemClass final
{
public:
	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR   applicationName = nullptr;
	HINSTANCE hinstance       = nullptr;
	HWND      hwnd            = nullptr;

	InputClass    *pInput     = nullptr;
	GraphicsClass *pGraphics  = nullptr;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static SystemClass* ApplicationHandle = 0;