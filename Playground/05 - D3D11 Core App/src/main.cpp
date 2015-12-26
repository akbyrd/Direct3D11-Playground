#include "stdafx.h"

#include "MessageQueue.h"
#include "HostWindow.h"
#include "RendererBase.h"
#include "GameTimer.h"

void ProcessMessage(Message&, GameTimer&, RendererBase&, const HostWindow&);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	//Enable run-time memory check for debug builds.
	#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	long ret = -1;

	//Create game components
	MessageQueue messageQueue;
	HostWindow window;
	RendererBase renderer;
	GameTimer gameTimer;

	//Initialize game components
	if ( !window.Initialize(L"Direct3D11 Playground", iCmdshow, 800, 600, messageQueue.GetQueuePusher()) ) { goto Cleanup; }
	if ( !renderer.Initialize(window.GetHWND()) ) { goto Cleanup; }
	gameTimer.Start();

	MSG msg = {};

	bool quit = false;

	//Message and render loop
	while ( !quit )
	{
		while ( PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) )
		{
			//Dispatch messages to the appropriate window
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

			//Clean quit
			if ( msg.message == WM_QUIT )
			{
				ret = msg.wParam;
				quit = true;
				break;
			}

			//I wonder what kind of messages wouldn't have a window pointer...
			if ( msg.hwnd == nullptr )
				Logging::Log(L"Found non-window message: " + std::to_wstring(msg.message));
		}
		if ( quit ) { break; }

		//Advance time
		gameTimer.Tick();

		//Process messages
		Message message;
		while ( messageQueue.PopMessage(message) ) {
			ProcessMessage(message, gameTimer, renderer, window);
		}

		//The fun stuff!
		if ( !renderer.Update(gameTimer) ) { goto Cleanup; }
	}

	//Cleanup and shutdown
Cleanup:
	renderer.Teardown();
	window.Teardown();

	return ret;
}

void ProcessMessage(Message& message, GameTimer &gameTimer, RendererBase &renderer, const HostWindow &window)
{
	switch ( message )
	{
	case Message::WindowResizingBegin:
		gameTimer.Stop();
		break;

	case Message::WindowResizingEnd:
		gameTimer.Start();
		break;

	case Message::WindowSizeChanged:
		//TODO: Handle return
		renderer.Resize();
		break;

	case Message::WindowMinimized:
		gameTimer.Stop();
		break;

	case Message::WindowUnminimized:
		gameTimer.Start();
		break;

	case Message::WindowActive:
		//Windows can be active while minimized. Silly.
		if ( window.IsActive() )
			gameTimer.Start();
		break;

	case Message::WindowInactive:
		gameTimer.Stop();
		break;

	case Message::Quit:
		//TODO: Handle in simulation class to begin shutdown process
		break;

	case Message::WindowClosed:
		//TODO: Handle if closing was unexpected
		break;
	}
}