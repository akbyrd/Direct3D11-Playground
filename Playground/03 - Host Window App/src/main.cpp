#include "stdafx.h"

#include "HostWindow.h"
#include "ExitCode.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	// Enable run-time memory check for debug builds.
	#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	//Create a message queue
	MessageQueue messageQueue;

	//Create the initialize the system object
	HostWindow window;
	RETURN_IF_FALSE(window.Initialize(L"Empty Window", iCmdshow, 800, 600, messageQueue.GetQueuePusher()), ExitCode::WindowInitializeFailed);

	int ret = 0;
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	//Message and render loop
	while ( ret == 0 )
	{
		//Handle thread messages
		while ( ret = PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) )
		{
			//Failure case
			if ( ret == -1 )
			{
				ret = ExitCode::PeekMessageFailed;
				LOG_ERROR(ret);
				break;
			}

			//Dispatch mesages to the appropriate window
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

			//I wonder what kind of messages wouldn't have a window pointer...
			if ( msg.hwnd == nullptr )
				Logging::Log(L"Found non-window message: " + msg.message);

			//Clean quit
			if ( msg.message == WM_QUIT ) { ret = ExitCode::Quit; break; }
		}

		//Slow the update rate when the window is not active
		if ( !window.IsActive() || window.IsMinimized() )
			Sleep(100);
	}

	//Teardown the window
	window.Teardown();

	return ret;
}