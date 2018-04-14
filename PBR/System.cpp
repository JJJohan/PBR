#include "System.h"
#include "Graphics.h"
#include "Input.h"

System::System() = default;

System::~System()
{
	ShutdownWindows();

	if (_pGraphics)
	{
		delete _pGraphics;
		_pGraphics = nullptr;
	}

	if (_pInput)
	{
		delete _pInput;
		_pInput = nullptr;
	}
}

bool System::Initialise()
{
	// Initialize the width and height of the screen to zero before sending the variables into the function.
	int screenWidth = 0;
	int screenHeight = 0;

	// Initialize the windows api.
	InitialiseWindows(screenWidth, screenHeight);

	// Create the input object.  This object will be used to handle reading the keyboard input from the user.
	_pInput = new Input;
	if (!_pInput)
	{
		return false;
	}

	// Initialize the input object.
	bool result = _pInput->Initialise(_pHinstance, _pHwnd, screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(_pHwnd, L"Could not initialize the input object.", L"Error", MB_OK);
		return false;
	}

	// Create the graphics object.  This object will handle rendering all the graphics for this application.
	_pGraphics = new Graphics;
	if (!_pGraphics)
	{
		return false;
	}

	// Initialize the graphics object.
	result = _pGraphics->Initialise(screenWidth, screenHeight, _pHwnd, _pInput);
	if (!result)
	{
		return false;
	}

	return true;
}

void System::Run() const
{
	MSG msg;

	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	bool done = false;
	while (!done)
	{
		// Handle the windows messages.
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			const bool result = Frame();
			if (!result)
			{
				done = true;
			}
		}

		// Check if the user pressed escape and wants to quit.
		if (_pInput->IsKeyDown(DIK_ESCAPE))
		{
			done = true;
		}
	}
}

bool System::Frame() const
{
	int mouseX, mouseY;

	// Do the input frame processing.
	bool result = _pInput->Frame();
	if (!result)
	{
		return false;
	}

	// Get the location of the mouse from the input object,
	_pInput->GetMouseLocation(mouseX, mouseY);

	// Do the frame processing for the graphics object.
	result = _pGraphics->Frame();
	if (!result)
	{
		return false;
	}

	// Finally render the graphics to the screen.
	result = _pGraphics->Render();
	if (!result)
	{
		return false;
	}

	return true;
}

LRESULT CALLBACK System::MessageHandler(const HWND hwnd, const UINT umsg, const WPARAM wparam,
                                        const LPARAM lparam) const
{
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

void System::InitialiseWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	// Get an external pointer to this object.
	ApplicationHandle = this;

	// Get the instance of this application.
	_pHinstance = GetModuleHandle(nullptr);

	// Give the application a name.
	_applicationName = L"Engine";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = _pHinstance;
	wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = HBRUSH(GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = _applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FullScreen)
	{
		screenWidth = 1920;
		screenHeight = 1200;

		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof dmScreenSettings);
		dmScreenSettings.dmSize = sizeof dmScreenSettings;
		dmScreenSettings.dmPelsWidth = static_cast<unsigned long>(screenWidth);
		dmScreenSettings.dmPelsHeight = static_cast<unsigned long>(screenHeight);
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	screenWidth = 1920;
	screenHeight = 1200;

	// Place the window in the middle of the screen.
	posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
	posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

	// Create the window with the screen settings and get the handle to it.
	_pHwnd = CreateWindowEx(WS_EX_APPWINDOW, _applicationName, _applicationName, WS_OVERLAPPEDWINDOW, posX, posY,
	                        screenWidth, screenHeight, nullptr, nullptr, _pHinstance, nullptr);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(_pHwnd, SW_SHOW);
	SetForegroundWindow(_pHwnd);
	SetFocus(_pHwnd);

	// Hide the mouse cursor.
	//ShowCursor(false);
}

void System::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FullScreen)
	{
		ChangeDisplaySettings(nullptr, 0);
	}

	// Remove the window.
	DestroyWindow(_pHwnd);
	_pHwnd = nullptr;

	// Remove the application instance.
	UnregisterClass(_applicationName, _pHinstance);
	_pHinstance = nullptr;

	// Release the pointer to this class.
	ApplicationHandle = nullptr;
}

LRESULT CALLBACK WndProc(const HWND hwnd, const UINT umessage, const WPARAM wparam, const LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
	case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		// All other messages pass to the message handler in the system class.
	default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}
