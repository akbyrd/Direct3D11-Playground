#pragma once

/* Notes:
 * Functions bodies inside a type are implicitly inline.
 * Only function bodies marked inline or static are allowed in headers.
 * Static functions outside of a type are local to the translation unit.
 */

struct Win32State
{
	HWND      hwnd                  = nullptr;
	i64       clockTicks            = 0;
	i64       clockTicksPerSecond   = 0;
	u16       mouseWheelAccumulator = 0;
	V2i       minClientSize         = {200, 200};
	V2i       clientSize            = {};
	bool      wasResized            = false;
	bool      isResizing            = false;
	SimMemory simMemory             = {};
};

#pragma region Foward Declarations
bool ClientSizeToWindowSize(V2i, DWORD, V2i*);
LRESULT CALLBACK StaticWndProc(HWND, u32, WPARAM, LPARAM);
LRESULT CALLBACK WndProc(Win32State*, HWND, u32, WPARAM, LPARAM);
#pragma endregion

bool
InitializeWindow(Win32State* win32State, HINSTANCE hInstance, LPCWSTR applicationName, i32 nCmdShow, V2i desiredClientSize)
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

	V2i windowPos  = {CW_USEDEFAULT, CW_USEDEFAULT};
	V2i windowSize = desiredClientSize;

	win32State->clientSize = desiredClientSize;

	DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION
		| WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
		| WS_SIZEBOX;

	ClientSizeToWindowSize(win32State->minClientSize, windowStyle, &win32State->minClientSize);
	ClientSizeToWindowSize(desiredClientSize, windowStyle, &windowSize);

	//Clamp size and center on desktop
	RECT usableDesktopRect = {};
	if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &usableDesktopRect, 0))
	{
		V2i usableDesktop = {
			usableDesktopRect.right - usableDesktopRect.left,
			usableDesktopRect.bottom - usableDesktopRect.top
		};

		Clamp(win32State->minClientSize, usableDesktop);

		i32 delta = windowSize.x - usableDesktop.x;
		if (delta > 0)
		{
			windowSize.x             -= delta;
			win32State->clientSize.x -= delta;
		}

		delta = windowSize.y - usableDesktop.y;
		if (delta > 0)
		{
			windowSize.y             -= delta;
			win32State->clientSize.y -= delta;
		}

		windowPos = (usableDesktop - windowSize) / 2;
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
		windowSize.x, windowSize.y,
		nullptr, nullptr,
		hInstance,
		win32State),
		IS_FALSE, LOG_LASTERROR(); return false);

	ShowWindow(win32State->hwnd, nCmdShow);

	return true;
}

inline bool
ClientSizeToWindowSize(V2i clientSize, DWORD windowStyle, V2i* windowSize)
{
	// TODO: It's pretty easy to create style where this function returns
	// different numbers than CreateWindow will. THis in turn can cause an
	// immediate resize/swap chain rebuild. Should place a strategic assert to
	// catch that case early.

	RECT windowRect = {0, 0, clientSize.x, clientSize.y};
	if (!AdjustWindowRect(&windowRect, windowStyle, false))
	{
		LOG_WARNING(L"Failed to translate client size to window size");
		return false;
	}

	windowSize->x = windowRect.right - windowRect.left;
	windowSize->y = windowRect.bottom - windowRect.top;

	return true;
}

inline bool
GetWindowClientSize(HWND hwnd, V2i* clientSize)
{
	RECT rect = {};
	if ( GetClientRect(hwnd, &rect) )
	{
		clientSize->x = rect.right - rect.left;
		clientSize->y = rect.bottom - rect.top;
	}
	else
	{
		HRESULT hr = GetLastError();
		if ( !LOG_HRESULT(hr) )
			LOG_WARNING(L"GetClientRect failed, but the last error passed a FAILED. HR = " + std::to_wstring(hr));
		return false;
	}

	return true;
}

LRESULT CALLBACK
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
	{
		return WndProc(win32State, hwnd, uMsg, wParam, lParam);
	}
	else
	{
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT CALLBACK
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

		case WM_MBUTTONDOWN:
		{
			input->mouseMiddle.isDown = true;
			input->mouseMiddle.transitionCount++;
			goto UpdateMousePosition;
		}

		case WM_MBUTTONUP:
		{
			input->mouseMiddle.isDown = false;
			input->mouseMiddle.transitionCount++;
			goto UpdateMousePosition;
		}

		case WM_MOUSEWHEEL:
		{
			//TODO: There's some drift here when scrolling quickly. Why?
			win32State->mouseWheelAccumulator += GET_WHEEL_DELTA_WPARAM(wParam);
			while ( win32State->mouseWheelAccumulator >= WHEEL_DELTA )
			{
				win32State->mouseWheelAccumulator -= WHEEL_DELTA;
				input->mouseWheelUp.transitionCount += 2;
			}
			while ( win32State->mouseWheelAccumulator <= -WHEEL_DELTA )
			{
				win32State->mouseWheelAccumulator += WHEEL_DELTA;
				input->mouseWheelDown.transitionCount += 2;
			}
			goto UpdateMousePosition;
		}


		UpdateMousePosition:
		case WM_MOUSEMOVE:
		{
			input->mouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
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
			minMaxInfo->ptMinTrackSize.x = win32State->minClientSize.x;
			minMaxInfo->ptMinTrackSize.y = win32State->minClientSize.y;
			return 0;
		}

		//Entering modal loop for move and resize events.
		case WM_ENTERSIZEMOVE:
		{
			win32State->isResizing = true;
			return 0;
		}

		//Exiting modal loop for move and resize events.
		case WM_EXITSIZEMOVE:
		{
			win32State->isResizing = false;

			V2i newClientSize;
			if (GetWindowClientSize(win32State->hwnd, &newClientSize))
			{
				lParam = MAKELPARAM(newClientSize.x, newClientSize.y);
				goto CheckForResize;
			}
			return 0;
		}

		CheckForResize:
		case WM_SIZE:
		{
			if ( !win32State->isResizing && wParam != SIZE_MINIMIZED )
			{
				V2i newClientSize = { LOWORD(lParam), HIWORD(lParam) };
				if (win32State->clientSize != newClientSize)
				{
					win32State->clientSize = newClientSize;
					win32State->wasResized = true;
				}
			}
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