#pragma once
#include "KeyEventHandler.h"

// Forward declaration
class Mario;

class MarioInputHandler : public CKeyEventHandler
{
private:
    Mario* mario;
public:
    MarioInputHandler(Mario* mario);
    void KeyState(BYTE* state) override;
    void OnKeyDown(int KeyCode) override;
    void OnKeyUp(int KeyCode) override;
};
