#include "stdafx.h"

#include "HostWindow.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	// Enable run-time memory check for debug builds.
	#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	bool success = false;

	//Create a message queue
	MessageQueue messageQueue;

	//Create the initialize the system object
	HostWindow window;
	success = window.Initialize(L"Empty Window", iCmdshow, 800, 600, messageQueue.GetQueuePusher());
	if ( !success ) { goto Cleanup; }

	long ret = 0;
	bool quit = false;
	MSG msg = {};

	//Message and render loop
	while ( !quit )
	{
		//Handle thread messages
		while ( PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) )
		{
			//Dispatch mesages to the appropriate window
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

			//I wonder what kind of messages wouldn't have a window pointer...
			if ( msg.hwnd == nullptr )
				Logging::Log(L"Found non-window message: " + msg.message);

			//Clean quit
			if ( msg.message == WM_QUIT )
			{
				ret = msg.wParam;
				quit = true;
				break;
			}
		}
		if ( quit ) { break; }

		//Slow the update rate when the window is not active
		if ( !window.IsActive() || window.IsMinimized() )
			Sleep(100);
	}

Cleanup:
	window.Teardown();

	return ret;
}