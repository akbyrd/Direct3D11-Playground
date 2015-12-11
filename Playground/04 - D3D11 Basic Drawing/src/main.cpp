#include "stdafx.h"

#include "MessageQueue.h"
#include "HostWindow.h"
#include "Renderer.h"
#include "Logging.h"
#include "GameTimer.h"
#include "ExitCode.h"

long ProcessMessage(Message&, GameTimer&, Renderer&, const HostWindow&);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	//Enable run-time memory check for debug builds.
	#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	long ret;

	//Create game components
	MessageQueue messageQueue;
	HostWindow window;
	Renderer renderer;
	GameTimer gameTimer;

	//Initialize game components
	ret = window.Initialize(L"Direct3D11 Playground", iCmdshow, 800, 600, messageQueue.GetQueuePusher()); CHECK_RET(ret);
	ret = renderer.Initialize(window.GetHWND()); CHECK_RET(ret);
	gameTimer.Start();

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	//Message and render loop
	bool quit = false;
	while ( ret == 0 )
	{
		while ( ret = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) )
		{
			if ( ret == -1 )
			{
				LOG_ERROR("PeekMessage failed");
				ret = ExitCode::PeekMessageFailed;
				goto Cleanup;
			}

			//Dispatch messages to the appropriate window
			TranslateMessage(&msg);
			DispatchMessage(&msg);

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
		while ( messageQueue.PopMessage(message) )
		{
			ret = ProcessMessage(message, gameTimer, renderer, window); CHECK_RET(ret);
		}

		//The fun stuff!
		ret = renderer.Update(gameTimer); CHECK_RET(ret);
	}

	//Cleanup and shutdown
Cleanup:
	renderer.Teardown();
	window.Teardown();

	return ret;
}

long ProcessMessage(Message& message, GameTimer &gameTimer, Renderer &renderer, const HostWindow &window)
{
	long ret = ExitCode::Success;

	switch ( message )
	{
	case Message::WindowResizingBegin:
		gameTimer.Stop();
		break;

	case Message::WindowResizingEnd:
		gameTimer.Start();
		break;

	case Message::WindowSizeChanged:
		ret = renderer.Resize();
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

	return ret;
}