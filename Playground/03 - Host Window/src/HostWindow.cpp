#include "stdafx.h"

#include "HostWindow.h"
#include "Utility.h"
#include "Logging.h"

using namespace Utility;

using Input = HostWindow::Input;

bool HostWindow::IsActive()    const { return isActive;    }
bool HostWindow::IsMinimized() const { return isMinimized; }
bool HostWindow::IsResizing()  const { return isResizing;  }
HWND HostWindow::GetHWND()     const { return hwnd;        }

bool HostWindow::Initialize(LPCWSTR applicationName, int iCmdshow,
                            uint16f clientWidth, uint16f clientHeight,
                            MessageQueue::Pusher* messageQueue)
{
	HostWindow::messageQueue = messageQueue;

	//Get the instance of this application
	hInstance = GetModuleHandleW(nullptr);

	//Give the application a name
	HostWindow::applicationName = applicationName;

	/* NOTE: style = CS_HREDRAW | CS_VREDRAW causes the window to go black
	 * during resizing. I wonder if it's sending messages than can be used
	 * to render the screen again?
	 */

	//Setup the windows class with default settings
	WNDCLASSEX wc = {};
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = nullptr;
	wc.lpszClassName = HostWindow::applicationName;
	wc.cbSize        = sizeof(wc);

	uint16f windowPosX = CW_USEDEFAULT;
	uint16f windowPosY = CW_USEDEFAULT;

	//Determine the usable area of the desktop
	RECT rect = {};
	if ( SystemParametersInfoW(SPI_GETWORKAREA, 0, &rect, 0) )
	{
		uint16f usableDesktopWidth  = rect.right - rect.left;
		uint16f usableDesktopHeight = rect.bottom - rect.top;

		ClientSizeToWindowSizeClamped(   minWidth,    minHeight, usableDesktopWidth, usableDesktopHeight, wc.style);
		ClientSizeToWindowSizeClamped(clientWidth, clientHeight, usableDesktopWidth, usableDesktopHeight, wc.style);

		//Center window
		windowPosX = (usableDesktopWidth  - clientWidth ) / 2;
		windowPosY = (usableDesktopHeight - clientHeight) / 2;
	}

	DWORD windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_SIZEBOX
	                  | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
	                  | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	//Create the window with the screen settings and get the handle to it
	WinCreateWindow(wc, WS_EX_APPWINDOW, applicationName, windowStyle,
	                windowPosX, windowPosY, clientWidth, clientHeight,
	                nullptr);

	//Bring the window up on the screen and set it as main focus
	ShowWindow(hwnd, iCmdshow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	return true;
}

bool HostWindow::ClientSizeToWindowSizeClamped(uint16f &width, uint16f &height,
                                               uint16f desktopWidth, uint16f desktopHeight,
                                               DWORD windowStyle)
{
	RECT windowRect = {};
	windowRect.left   = 0;
	windowRect.top    = 0;
	windowRect.right  = width;
	windowRect.bottom = height;

	BOOL ret = AdjustWindowRect(&windowRect, windowStyle, false);
	if ( !ret )
	{
		LOG_WARNING(L"Failed to translate client size to window size");
		return false;
	}

	width  = windowRect.right - windowRect.left;
	height = windowRect.bottom - windowRect.top;

	//Clamp window size to fit screen
	width  = std::min(width , desktopWidth);
	height = std::min(height, desktopHeight);

	return true;
}

Input* HostWindow::GetInput()
{
	//Copy
	previousInput = input;

	//Clear
	for ( size_t i = 0; i < ArraySize(input.buttons); ++i )
	{
		input.buttons[i].transitionCount = 0;
	}

	//Return
	return &previousInput;
}

void HostWindow::Teardown()
{
	//Show the mouse cursor
	ShowCursor(true);

	//Remove the window
	DestroyWindow(hwnd);
	hwnd = nullptr;

	//Remove the application instance
	UnregisterClassW(applicationName, hInstance);
	hInstance = nullptr;
}

LRESULT HostWindow::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//throw_assert
	switch ( uMsg )
	{
		case WM_KEYUP:
		case WM_KEYDOWN: {
			if ( wParam == VK_ESCAPE )
				PostQuitMessage(0);
			return 0;
		}

		case WM_ACTIVATE: {
			if ( LOWORD(wParam) == WA_INACTIVE )
			{
				isActive = false;
				messageQueue->PushMessage(Message::WindowInactive);
			}
			else
			{
				isActive = true;
				messageQueue->PushMessage(Message::WindowActive);
			}
			return 0;
		}

		//Entering modal loop for move and resize events.
		case WM_ENTERSIZEMOVE: {
			isResizing = true;
			messageQueue->PushMessage(Message::WindowResizingBegin);
			return 0;
		}

		//Exiting modal loop for move and resize events.
		case WM_EXITSIZEMOVE: {
			isResizing = false;
			messageQueue->PushMessage(Message::WindowResizingEnd);
			messageQueue->PushMessage(Message::WindowSizeChanged);
			return 0;
		}

		case WM_SIZE: {
			switch ( wParam )
			{
				case SIZE_MINIMIZED: {
					isMinimized = true;
					messageQueue->PushMessage(Message::WindowMinimized);
					break;
				}

				case SIZE_MAXIMIZED: {
					isMaximized = true;
					messageQueue->PushMessage(Message::WindowSizeChanged);
					break;
				}

				/* Sent for any size event that isn't covered above, doesn't mean an
				 * actual 'restore' operation on the window.
				*/
				case SIZE_RESTORED: {
					//Actual restore operation
					if ( isMinimized )
					{
						isMinimized = false;
						messageQueue->PushMessage(Message::WindowUnminimized);
					}
					//Actual restore operation
					else if ( isMaximized )
					{
						isMaximized = false;
						messageQueue->PushMessage(Message::WindowSizeChanged);
					}
					//Just a resize operation
					else if ( !isResizing )
					{
						messageQueue->PushMessage(Message::WindowSizeChanged);
					}
					break;
				}
			}
			return 0;
		}

		case WM_GETMINMAXINFO: {
			//Limit the minimum window size
			((MINMAXINFO*) lParam)->ptMinTrackSize.x = minWidth;
			((MINMAXINFO*) lParam)->ptMinTrackSize.y = minHeight;
			return 0;
		}

		//Recieved unhandled keystroke while a menu is active.
		case WM_MENUCHAR: {
			UINT menuType = HIWORD(wParam);
			bool isSystem = menuType == MF_SYSMENU;

			UINT keyPressed = LOWORD(wParam);
			bool enterPressed = keyPressed == VK_RETURN;

			//Don't beep when exiting fullscreen with Alt+Enter
			if ( isSystem && enterPressed )
				return MAKELRESULT(0, MNC_CLOSE);

			break;
		}

		case WM_CLOSE: {
			PostQuitMessage(1);
			return 0;
		}

		case WM_DESTROY: {
			messageQueue->PushMessage(Message::WindowClosed);
			return 0;
		}

		case WM_MOUSEWHEEL: {
			//TODO: There's some drift here when scrolling quickly. Why?
			mouseWheelAccumulator += GET_WHEEL_DELTA_WPARAM(wParam);
			while ( mouseWheelAccumulator >= WHEEL_DELTA )
			{
				mouseWheelAccumulator -= WHEEL_DELTA;
				input.mouseWheelUp.transitionCount += 2;
			}
			 while ( mouseWheelAccumulator <= -WHEEL_DELTA )
			{
				mouseWheelAccumulator += WHEEL_DELTA;
				input.mouseWheelDown.transitionCount += 2;
			}
			goto UpdateMousePosition;
		}

		case WM_RBUTTONDOWN: {
			input.mouseRight.isDown = true;
			input.mouseRight.transitionCount++;
			goto UpdateMousePosition;
		}

		case WM_RBUTTONUP: {
			input.mouseRight.isDown = false;
			input.mouseRight.transitionCount++;
			goto UpdateMousePosition;
		}

		case WM_LBUTTONDOWN: {
			input.mouseLeft.isDown = true;
			input.mouseLeft.transitionCount++;
			goto UpdateMousePosition;
		}

		case WM_LBUTTONUP: {
			input.mouseLeft.isDown = false;
			input.mouseLeft.transitionCount++;
			goto UpdateMousePosition;
		}

		case WM_MBUTTONDOWN: {
			input.mouseMiddle.isDown = true;
			input.mouseMiddle.transitionCount++;
			goto UpdateMousePosition;
		}

		case WM_MBUTTONUP: {
			input.mouseMiddle.isDown = false;
			input.mouseMiddle.transitionCount++;
			goto UpdateMousePosition;
		}

		/* TODO: Only the final mouse position is taken into account, rather than
		 * its movement over the course of the frame. Make this better?
		 */
		UpdateMousePosition:
		case WM_MOUSEMOVE: {
			input.mouseX = GET_X_LPARAM(lParam);
			input.mouseY = GET_Y_LPARAM(lParam);
			return 0;
		}
	}

	//Send unhandled messages to the base class
	return __super::MessageHandler(uMsg, wParam, lParam);
}