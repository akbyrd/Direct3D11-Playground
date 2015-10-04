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

	//TODO: Clean up properly on failure cases, instead of returning immediately.

	//Init a window
	HostWindow* window = new HostWindow();
	RETURN_IF_FALSE(window, ExitCode::WindowAllocFailed);
	RETURN_IF_FAILED(window->Initialize());

	//Init a renderer
	Renderer* renderer = new Renderer(window->GetHWND());
	RETURN_IF_FALSE(renderer, ExitCode::RendererAllocFailed);
	RETURN_IF_FAILED(renderer->Initialize());

	int ret = 0;
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	//Message and render loop
	while ( ret == 0 )
	{
		while ( ret = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) )
		{
			if ( ret == -1 )
			{
				ret = ExitCode::PeekMessageFailed;
				break;
			}

			//Dispatch messages to the appropriate window
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			//I wonder what kind of messages wouldn't have a window pointer...
			if ( msg.hwnd == nullptr )
				Logging::Log(L"Found non-window message: " + std::to_wstring(msg.message));

			//Clean quit
			if ( msg.message == WM_QUIT ) { ret = ExitCode::Quit; break; }
		}
		if ( ret != 0 ) { break; }

		//The fun stuff!
		if (   window->Update() < 0 ) { break; }
		if ( renderer->Update() < 0 ) { break; }
	}

	//Cleanup and shutdown
	renderer->Teardown();
	delete renderer;
	renderer = nullptr;

	window->Teardown();
	delete window;
	window = nullptr;

	return ret;
}