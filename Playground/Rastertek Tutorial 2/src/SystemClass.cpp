#include "stdafx.h"
#include "SystemClass.h"

bool SystemClass::Initialize()
{
	int screenWidth = 0;
	int screenHeight = 0;

	//Initialize the Windows API
	InitializeWindows(screenWidth, screenHeight);

	//Initialize input
	RETURN_IF_FALSE(pInput = new InputClass, false);
	RETURN_IF_FALSE(pInput->Initialize()   , false);

	//Initialize graphics
	RETURN_IF_FALSE(pGraphics = new GraphicsClass                         , false);
	RETURN_IF_FALSE(pGraphics->Initialize(screenWidth, screenHeight, hwnd), false);

	return true;
}

void SystemClass::Shutdown()
{
	//Release graphics
	if ( pGraphics )
	{
		pGraphics->Shutdown();
		delete pGraphics;
		pGraphics = nullptr;
	}

	//Release input
	if ( pInput )
	{
		delete pInput;
		pInput = nullptr;
	}

	//Shutdown the window
	ShutdownWindows();
}

void SystemClass::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	//Loop until there is a quit message from the window or user
	while ( true )
	{
		//Handle the windows messages
		if ( PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			//If Windwos signals to end the application, exit
			if ( msg.message == WM_QUIT ) { break; }
		}

		//Otherwise do the frame processing
		if ( !Frame() ) { break; }
	}

	return;
}

bool SystemClass::Frame()
{
	//Check if the user pressed escape and wants to exit the application
	if ( pInput->IsKeyDown(VK_ESCAPE) ) { return false; }

	//Do the frame processing for the graphics object
	RETURN_IF_FALSE(pGraphics->Frame(), false);

	return true;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch ( umsg )
	{
	case WM_KEYDOWN:
		pInput->KeyDown((unsigned int) wparam);
		return 0;

	case WM_KEYUP:
		pInput->KeyUp((unsigned int) wparam);
		return 0;

	//Send unhandled messages to the default handler
	default:
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}

void SystemClass::InitializeWindows(int& windowWidth, int& windowHeight)
{
	//Get an external pointer to this object
	ApplicationHandle = this;

	//Get the instance of this application
	hinstance = GetModuleHandle(nullptr);

	//Give the application a name
	applicationName = L"Direct3D11 Playground";

	//Setup the windows class with default settings
	WNDCLASSEX wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hinstance;
	wc.hIcon         = LoadIcon(nullptr, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = nullptr;
	wc.lpszClassName = applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);

	//Register the window class
	RegisterClassEx(&wc);

	//Determine the resolution of the desktop screen
	int desktopWidth  = GetSystemMetrics(SM_CXSCREEN);
	int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

	//Determine window size
	windowWidth  = desktopWidth  > 800 ? 800 : desktopWidth;
	windowHeight = desktopHeight > 600 ? 600 : desktopHeight;

	//Determine window position
	int windowPosX = (desktopWidth  - windowWidth ) / 2;
	int windowPosY = (desktopHeight - windowHeight) / 2;

	//Create the window with the screen settings and get the handle to it
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName,
	                      WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
	                      windowPosX, windowPosY, windowWidth, windowHeight,
	                      nullptr, nullptr, hinstance, nullptr);

	//Bring the window up on the screen and set it as main focus
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
}

void SystemClass::ShutdownWindows()
{
	//Show the mouse cursor
	ShowCursor(true);

	//Remove the window
	DestroyWindow(hwnd);
	hwnd = nullptr;

	//Remove the application instance
	UnregisterClass(applicationName, hinstance);
	hinstance = nullptr;

	//Release the pointer to this class
	ApplicationHandle = nullptr;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch ( umessage )
	{
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	default:
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
}