#pragma once

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

class Input
{
public:
	Input();
	~Input();

	bool Initialise(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	bool Frame();

	bool IsKeyDown(int keyCode);
	void GetMouseLocation(int& x, int& y) const;
	void GetMouseDelta(int& x, int& y) const;

private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

	IDirectInput8* _mDirectInput;
	IDirectInputDevice8* _mKeyboard;
	IDirectInputDevice8* _mMouse;

	unsigned char _mKeyboardState[256];
	DIMOUSESTATE _mMouseState;

	int _mScreenWidth, _mScreenHeight;
	int _mouseDeltaX, _mouseDeltaY;
	int _mMouseX, _mMouseY;
};
