#pragma once
#include <Windows.h>
#include <math.h>
#include "../render/Sprites.h"

class Intro
{
private:
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
    void OnKeyDown(int KeyCode);
    void OnKeyUp(int KeyCode);
    bool IsDone() { return isDone; }
    int GetSelectedOption() { return 1; } // Mặc định trả về 1 (Game Start) cho các hệ thống khác nếu cần
};