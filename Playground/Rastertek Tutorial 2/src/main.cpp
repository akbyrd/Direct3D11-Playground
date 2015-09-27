#include "stdafx.h"
#include "SystemClass.h"

//TODO: Research smart pointers

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	//Create the initialize the system object
	SystemClass* System;
	RETURN_IF_FALSE(System = new SystemClass, -1);
	RETURN_IF_FALSE(System->Initialize()    , -1);

	//Run, baby, run!
	System->Run();

	//Shutdown and release the system object
	System->Shutdown();
	delete System;
	System = nullptr;

	return 0;
}