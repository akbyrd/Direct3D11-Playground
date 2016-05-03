#include <Windows.h>

#include "win32_window.hpp"
#include "Platform.h"
#include "Simulation.h"

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int32 nCmdShow)
{
	//TODO: 64-bit build?
	SimMemory simMemory = {};
	simMemory.size = Megabyte;


	//Initialize
	{
		IF( simMemory.bytes = VirtualAlloc(0, simMemory.size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE),
			IS_FALSE, LOG_LASTERROR(); goto Failure);

		IF( QueryPerformanceFrequency((LARGE_INTEGER*) &simMemory.tickFrequency),
			IS_FALSE, LOG_LASTERROR());

		//TODO: Get window size from sim?
		InitializeSimulation(&simMemory);

		Window2 window = {};
		window.inputQueue = &simMemory.input;
		IF( InitializeWindow(&window, hInstance, L"D3D11 Playground", nCmdShow, {800, 600}),
			IS_FALSE, goto Failure);
	}


	//Main loop
	{
		LARGE_INTEGER lastTicks;
		IF( QueryPerformanceCounter(&lastTicks),
		   IS_FALSE, LOG_LASTERROR());

		bool quit = false;
		while (!quit)
		{
			MSG msg = {};

			while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				//TranslateAccelerator?
				TranslateMessage(&msg);
				DispatchMessageW(&msg);

				if (msg.message == WM_QUIT)
				{
					Logging::Log(L"WM_QUIT recieved");
					quit = true;
					break;
				}
			}
			if (quit) { break; }

			LARGE_INTEGER currentTicks;
			IF( QueryPerformanceCounter(&currentTicks),
				IS_FALSE, LOG_LASTERROR());

			//NOTE: Ticks can be negative if the process is shuffled
			uint64 newTicks = currentTicks.QuadPart - lastTicks.QuadPart;
			if (newTicks < 0) { newTicks = 0; }
			lastTicks = currentTicks;

			//TODO: What if the simulation wants to quit?
			UpdateSimulation(&simMemory, newTicks);
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