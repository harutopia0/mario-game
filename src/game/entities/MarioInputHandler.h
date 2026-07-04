#pragma once
#include "engine/input/KeyEventHandler.h"

// Forward declaration
class Mario;

class MarioInputHandler : public CKeyEventHandler
{
private:
    Mario* mario;
public:
    MarioInputHandler(Mario* mario);
    void KeyState(BYTE* state) override;
};
