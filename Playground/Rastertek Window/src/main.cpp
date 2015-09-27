#include "stdafx.h"
#include "Window.h"

//TODO: Research smart pointers

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	//Create the initialize the system object
	Window* window;
	RETURN_IF_FALSE(window = new Window , -1);
	RETURN_IF_FALSE(window->Initialize(), -1);

	//Run, baby, run!
	window->Run();

	//Shutdown and release the system object
	window->Shutdown();
	delete window;
	window = nullptr;

	return 0;
}