#include <Windows.h>
#include <windowsx.h>

//TODO: IGNORE macro redefinition
#include "Platform.h"
#include "win32_window.hpp"
#include "Simulation.hpp"
#include "Renderer.hpp"

#pragma region Foward Declarations
void UpdateFrameStatistics(RendererState*, r64);
#pragma endregion

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, i32 nCmdShow)
{
	//Enable run-time memory check for debug builds.
	#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif


	//TODO: 64-bit build?
	Win32State win32State = {};
	win32State.simMemory.size = Megabyte;

	SimMemory*     simMemory     = &win32State.simMemory;
	InputState*    input         = &win32State.simMemory.input;
	RendererState* rendererState = nullptr;


	//Initialize
	{
		IF( simMemory->bytes = VirtualAlloc(0, simMemory->size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE),
			IS_FALSE, LOG_LASTERROR(); goto Failure);

		IF( QueryPerformanceFrequency((LARGE_INTEGER*) &win32State.clockTicksPerSecond),
			IS_FALSE, LOG_LASTERROR());

		//TODO: Get window size from sim?
		V2i windowSize = {};
		InitializeSimulation(simMemory, &windowSize);

		IF( InitializeWindow(&win32State, hInstance, L"D3D11 Playground", nCmdShow, {800, 600}),
			IS_FALSE, goto Failure);

		//TODO: Formally define memory chunks
		//TODO: Probably put rendererState on the stack and just it's memory in the simMemory
		rendererState = (RendererState*) simMemory->bytes;
		simMemory->bytes  = rendererState + 1;
		simMemory->size  -= sizeof(*rendererState);
		*rendererState = {};

		rendererState->hwnd = win32State.hwnd;
		IF( InitializeRenderer(rendererState),
			IS_FALSE, goto Failure);
	}


	//Main loop
	{
		i64 maxTickPerFrame = (i64) (.5f * win32State.clockTicksPerSecond);

		i64 previousTicks;
		IF( QueryPerformanceCounter((LARGE_INTEGER*) &previousTicks),
			IS_FALSE, LOG_LASTERROR());

		bool quit = false;
		while (!quit)
		{
			MSG msg = {};

			//TODO: Log WndProc calls that happen outside this pump?
			while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				//TranslateAccelerator?
				TranslateMessage(&msg);
				DispatchMessageW(&msg);

				if (msg.message == WM_QUIT)
				{
					Logging::Log(L"WM_QUIT received");
					quit = true;
					break;
				}
			}
			if (quit) { break; }

			if (win32State.wasResized)
			{
				win32State.wasResized = false;
				ResizeRenderer(rendererState);
			}

			i64 currentTicks;
			IF( QueryPerformanceCounter((LARGE_INTEGER*) &currentTicks),
				IS_FALSE, LOG_LASTERROR());

			//NOTE: Ticks can be negative if the process is shuffled
			i64 newTicks = Clamp(currentTicks - previousTicks, 0LL, maxTickPerFrame);

			win32State.clockTicks += newTicks;
			previousTicks = currentTicks;

			input->t  = (r64) win32State.clockTicks / (r64) win32State.clockTicksPerSecond;
			input->dt = (r64)              newTicks / (r64) win32State.clockTicksPerSecond;

			//TODO: What if the simulation wants to quit?
			UpdateSimulation(simMemory);
			Render(rendererState, input->t);

			for (size_t i = 0; i < ArrayCount(input->buttons); ++i)
				input->buttons[i].transitionCount = 0;

			UpdateFrameStatistics(rendererState, input->t);
		}
	}


Cleanup:
	/* NOTE: Windows automatically frees memory on exit.
	 * LEAK ALL THE THINGS!
	 */

	TeardownRenderer(rendererState);
	return 0;

Failure:
	__debugbreak();
	goto Cleanup;
}

void
UpdateFrameStatistics(RendererState* state, r64 t)
{
	static const int bufferSize = 30;

	static char titleBuffer[128];
	static double buffer[bufferSize];
	static int head = -1;
	static int length = 0;
	static double deltaToMS;

	//HACK: Hate this
	if ( length == 0 )
		buffer[bufferSize - 1] = t;

	//Update the head position and length
	head = (head + 1) % bufferSize;
	if ( length < bufferSize - 1 )
	{
		++length;
		deltaToMS = 1000. / length;
	}

	//Update the head value
	buffer[head] = t;

	int tail = (head - length) % bufferSize;
	if ( tail < 0 )
		tail += bufferSize;

	//Update FPS in window title periodically
	static double lastFPSUpdateTime = 0;
	if ( t - lastFPSUpdateTime >= .5 )
	{
		lastFPSUpdateTime = t;

		r64 delta = buffer[head] - buffer[tail];
		r64 averageFrameTime = delta * deltaToMS;

		snprintf(titleBuffer, ArraySize(titleBuffer),
				 "FPS: %.0f   Frame Time: %.4f ms (%d x %d)",
				 1000 / averageFrameTime,
				 averageFrameTime,
				 state->renderSize.x,
				 state->renderSize.y
		);

		SetWindowTextA(state->hwnd, titleBuffer);
	}

	return;
}