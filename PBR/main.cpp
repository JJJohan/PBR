#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "System.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	// Create the system object.
	System* system = new System;
	if (!system)
	{
		return 0;
	}

	// Initialize and run the system object.
	if (system->Initialise())
	{
		system->Run();
	}

	// Shutdown and release the system object.
	delete system;
	system = nullptr;

	return 0;
}
