#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <Windows.h>
#include "HostWindow.h"
#include "ExitCode.h"

#define RETURN_IF_FALSE(x, r) { if ( !(x) ) { return r; } }

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	// Enable run-time memory check for debug builds.
	#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	//Create the initialize the system object
	HostWindow* window;
	RETURN_IF_FALSE(window = new HostWindow, -1);
	RETURN_IF_FALSE(window->Initialize()   , -1);

	//Run, baby, run!
	window->Run();

	//Shutdown and release the system object
	window->Shutdown();
	delete window;
	window = nullptr;

	return 0;
}