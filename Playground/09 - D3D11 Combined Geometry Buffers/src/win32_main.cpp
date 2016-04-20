#include <Windows.h>

#include "win32_window.h"
#include "Logging.h"
#include "Platform.h"
#include "Simulation.h"

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int32 nCmdShow)
{
	InputQueue inputQueue = {};


	//Initialize
	{
		//TODO: 64-bit build
		uint32 requiredMemory = Meagbyte;

		void* memoryPool;
		IF( memoryPool = VirtualAlloc(0, requiredMemory, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE),
			IS_FALSE, LOG_LASTERROR(); goto Failure);

		LARGE_INTEGER tickFrequency;
		IF( QueryPerformanceFrequency(&tickFrequency),
			IS_FALSE, LOG_LASTERROR());

		//TODO: Get window size from sim?
		InitializeSimulation(memoryPool, tickFrequency.QuadPart);

		Window2 window = {};
		IF( InitializeWindow(&window, &inputQueue, hInstance, L"D3D11 Playground", nCmdShow, SIZE{800, 600}),
			IS_FALSE, goto Failure);
	}


	//Main loop
	{
		bool quit = false;
		LARGE_INTEGER lastTicks = {};

		while ( !quit )
		{
			MSG msg = {};

			while ( PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) )
			{
				//TranslateAccelerator?
				TranslateMessage(&msg);
				DispatchMessageW(&msg);

				if ( msg.message == WM_QUIT )
				{
					
					Logging::Log(L"Quit message recieved");
					quit = true;
					break;
				}
			}
			if ( quit ) { break; }

			LARGE_INTEGER currentTicks = {};
			IF( QueryPerformanceCounter(&currentTicks),
				IS_FALSE, LOG_LASTERROR());

			uint64 newTicks = currentTicks.QuadPart - lastTicks.QuadPart;
			lastTicks = currentTicks;

			//TODO: Who creates/calls the renderer?
			//TODO: What if the simulation wants to quit?
			//TODO: Put ticks in the queue
			UpdateSimulation(newTicks, inputQueue);
		}
	}

Cleanup:
	/* NOTE: Windows automatically frees memory on exit.
	 * LEAK ALL THE THINGS!
	 */

	return 0;

Failure:
	__debugbreak();
	goto Cleanup;
}