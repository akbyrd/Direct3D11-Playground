#include "stdafx.h"
#include "Renderer.h"
#include "HostWindow.h"

//TODO: Alternative to DXTrace? Fancy message box and debugging prompt
//TODO: Primary question: how to get messages from e.g. window to handle or pass to other objects?
//TODO: Decide how to handle the timer (should probably be in here, but we need to get signals from the 
//TODO: Decide how to handle resizing
//TODO: Input

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	//Enable run-time memory check for debug builds.
	#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	long ret = 0;

	//Create a window
	HostWindow* window = new HostWindow();
	if ( !window )
	{
		LOG_ERROR("HostWindow allocation failed");
		ret = ExitCode::WindowAllocFailed;
		goto Cleanup;
	}

	//Init window
	ret = window->Initialize(iCmdshow); CHECK_RET(ret);

	//Create a renderer
	Renderer* renderer = new Renderer();
	if ( !renderer )
	{
		LOG_ERROR("Renderer allocation failed");
		ret = ExitCode::RendererAllocFailed;
		goto Cleanup;
	}

	//Init renderer
	ret = renderer->Initialize(window->GetHWND()); CHECK_RET(ret);

	//Create a timer
	GameTimer* gameTimer = new GameTimer();
	if ( !gameTimer )
	{
		LOG_ERROR("GameTimer allocation failed");
		ret = ExitCode::TimerAllocFailed;
		goto Cleanup;
	}

	//Init timer
	gameTimer->Start();

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

		//The fun stuff!
		gameTimer->Tick();
		ret =   window->Update();          CHECK_RET(ret);
		ret = renderer->Update(gameTimer); CHECK_RET(ret);

		//Sleep(1);
	}

	//Cleanup and shutdown
Cleanup:
	if ( gameTimer )
	{
		delete gameTimer;
		gameTimer = nullptr;
	}

	if ( renderer )
	{
		renderer->Teardown();
		delete renderer;
		renderer = nullptr;
	}

	if ( window )
	{
		window->Teardown();
		delete window;
		window = nullptr;
	}

	return ret;
}