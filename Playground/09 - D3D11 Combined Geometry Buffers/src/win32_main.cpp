#include <Windows.h>

#include "win32_window.hpp"
#include "Platform.h"
#include "Simulation.h"

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int32 nCmdShow)
{
	//TODO: 64-bit build
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
		IF( InitializeWindow(&window, &simMemory.input, hInstance, L"D3D11 Playground", nCmdShow, SIZE{800, 600}),
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
					
					Logging::Log(L"Quit message recieved");
					quit = true;
					break;
				}
			}
			if (quit) { break; }

			LARGE_INTEGER currentTicks;
			IF( QueryPerformanceCounter(&currentTicks),
				IS_FALSE, LOG_LASTERROR());

			simMemory.ticks += currentTicks.QuadPart - lastTicks.QuadPart;
			lastTicks = currentTicks;

			//TODO: What if the simulation wants to quit?
			UpdateSimulation(&simMemory);
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