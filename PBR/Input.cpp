#include "Input.h"

Input::Input() = default;

Input::~Input()
{
	// Release the mouse.
	if (_mMouse)
	{
		_mMouse->Unacquire();
		_mMouse->Release();
		_mMouse = nullptr;
	}

	// Release the keyboard.
	if (_mKeyboard)
	{
		_mKeyboard->Unacquire();
		_mKeyboard->Release();
		_mKeyboard = nullptr;
	}

	// Release the main interface to direct input.
	if (_mDirectInput)
	{
		_mDirectInput->Release();
		_mDirectInput = nullptr;
	}
}

bool Input::Initialise(const HINSTANCE hinstance, const HWND hwnd, const int screenWidth, const int screenHeight)
{
	// Store the screen size which will be used for positioning the mouse cursor.
	_mScreenWidth = screenWidth;
	_mScreenHeight = screenHeight;

	// Initialize the location of the mouse on the screen.
	_mMouseX = 0;
	_mMouseY = 0;

	// Initialize the main direct input interface.
	HRESULT result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
	                                    reinterpret_cast<void**>(&_mDirectInput), nullptr);
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the direct input interface for the keyboard.
	result = _mDirectInput->CreateDevice(GUID_SysKeyboard, &_mKeyboard, nullptr);
	if (FAILED(result))
	{
		return false;
	}

	// Set the data format.  In this case since it is a keyboard we can use the predefined data format.
	result = _mKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}

	// Set the cooperative level of the keyboard to not share with other programs.
	result = _mKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	// Now acquire the keyboard.
	result = _mKeyboard->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the direct input interface for the mouse.
	result = _mDirectInput->CreateDevice(GUID_SysMouse, &_mMouse, nullptr);
	if (FAILED(result))
	{
		return false;
	}

	// Set the data format for the mouse using the pre-defined mouse data format.
	result = _mMouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}

	// Set the cooperative level of the mouse to share with other programs.
	result = _mMouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	// Acquire the mouse.
	result = _mMouse->Acquire();
	return !FAILED(result);
}

bool Input::Frame()
{
	// Read the current state of the keyboard.
	bool result = ReadKeyboard();
	if (!result)
	{
		return false;
	}

	// Read the current state of the mouse.
	result = ReadMouse();
	if (!result)
	{
		return false;
	}

	// Process the changes in the mouse and keyboard.
	ProcessInput();

	return true;
}

bool Input::ReadKeyboard()
{
	// Read the keyboard device.
	const HRESULT result = _mKeyboard->GetDeviceState(sizeof _mKeyboardState, LPVOID(&_mKeyboardState));
	if (FAILED(result))
	{
		// If the keyboard lost focus or was not acquired then try to get control back.
		if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)
		{
			_mKeyboard->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool Input::ReadMouse()
{
	// Read the mouse device.
	const HRESULT result = _mMouse->GetDeviceState(sizeof(DIMOUSESTATE), LPVOID(&_mMouseState));
	if (FAILED(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)
		{
			_mMouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

void Input::ProcessInput()
{
	// Update the location of the mouse cursor based on the change of the mouse location during the frame.
	_mouseDeltaX = _mMouseState.lX;
	_mouseDeltaY = _mMouseState.lY;
	_mMouseX += _mouseDeltaX;
	_mMouseY += _mouseDeltaY;

	// Ensure the mouse location doesn't exceed the screen width or height.
	if (_mMouseX < 0) { _mMouseX = 0; }
	if (_mMouseY < 0) { _mMouseY = 0; }

	if (_mMouseX > _mScreenWidth) { _mMouseX = _mScreenWidth; }
	if (_mMouseY > _mScreenHeight) { _mMouseY = _mScreenHeight; }
}

bool Input::IsKeyDown(const int keyCode)
{
	// Do a bitwise and on the keyboard state to check if the key is currently being pressed.
	return (_mKeyboardState[keyCode] & 0x80) != 0;
}

void Input::GetMouseLocation(int& x, int& y) const
{
	x = _mMouseX;
	y = _mMouseY;
}

void Input::GetMouseDelta(int& x, int& y) const
{
	x = _mouseDeltaX;
	y = _mouseDeltaY;
}
