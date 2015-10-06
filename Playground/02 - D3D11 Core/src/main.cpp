#include "stdafx.h"
#include "Renderer.h"
#include "HostWindow.h"
#include "Logging.h"

//TODO: Proper cleanup flow

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
		LOG_ERROR(L"HostWindow allocation failed");
		ret = ExitCode::WindowAllocFailed;
		goto Cleanup;
	}

	//Init window
	ret = window->Initialize(); CHECK_RET(ret);

	//Create a renderer
	Renderer* renderer = new Renderer(window->GetHWND());
	if ( !renderer )
	{
		LOG_ERROR(L"Renderer allocation failed");
		ret = ExitCode::RendererAllocFailed;
		goto Cleanup;
	}

	//Init renderer
	ret = renderer->Initialize(); CHECK_RET(ret);

	//Create a timer
	//GameTimer* gameTimer = new GameTimer();
	//if ( !gameTimer )
	//{
	//	LOG_ERROR(L"GameTimer allocation failed");
	//	ret = ExitCode::TimerAllocFailed;
	//	goto Cleanup;
	//}

	//Init timer
	//gameTimer->Reset();

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	//Message and render loop
	while ( ret == 0 )
	{
		while ( ret = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) )
		{
			if ( ret == -1 )
			{
				LOG_ERROR(L"PeekMessage failed");
				ret = ExitCode::PeekMessageFailed;
				goto Cleanup;
			}

			//Dispatch messages to the appropriate window
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			//I wonder what kind of messages wouldn't have a window pointer...
			if ( msg.hwnd == nullptr )
				Logging::Log(L"Found non-window message: " + std::to_wstring(msg.message));

			//Clean quit
			if ( msg.message == WM_QUIT )
			{
				Logging::Log(L"Quitting");
				ret = ExitCode::Quit;
				break;
			}
		}
		if ( ret != 0 ) { break; }

		//The fun stuff!
		//gameTimer->Tick();
		ret =   window->Update(); CHECK_RET(ret);
		ret = renderer->Update(); CHECK_RET(ret);

		//TODO: Looks like the frame time is totally wrong until the buffer fills
		Sleep(100);
	}

	//Cleanup and shutdown
Cleanup:

	//if ( gameTimer )
	//{
	//	delete gameTimer;
	//	gameTimer = nullptr;
	//}

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