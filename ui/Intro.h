#pragma once
#include <Windows.h>
#include <math.h>
#include "../render/Sprites.h"

class Intro
{
private:
    int currentOption; // 1: 1 Player, 2: 2 Player
    bool isDone;

    float curtainY;
    float curtainSpeed;
    bool isCurtainUp;
    float curtainHeight;

    float logoY;
    float logoSpeed;
    bool isLogoDown;

    // Các biến cho hiệu ứng nhấp nháy số 3
    DWORD timerNum3;
    int stateNum3;

public:
    Intro();
    void LoadSprites();
    void Update(DWORD dt);
    void Render();
    bool IsDone() { return isDone; }
    int GetSelectedOption() { return currentOption; }
};