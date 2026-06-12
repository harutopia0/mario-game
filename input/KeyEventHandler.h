#pragma once

#include <Windows.h>

/*
	Abstract class to define keyboard event handlers
*/
class CKeyEventHandler
{
public:
	virtual void KeyState(BYTE* state) = 0;
};

typedef CKeyEventHandler* LPKEYEVENTHANDLER;
