#pragma once
#include <Windows.h>
#include "../render/Sprites.h"

class Intro
{
private:
    int currentOption; // 1: 1 Player, 2: 2 Player
    bool isDone;

public:
    Intro();
    void Update(DWORD dt);
    void Render();
    bool IsDone() { return isDone; }
    int GetSelectedOption() { return currentOption; }
};