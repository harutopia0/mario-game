#pragma once
#include <string>
#include <Windows.h>
#include "../render/Sprites.h"

class HUD
{
private:
    static HUD* __instance;

    int time;
    DWORD timeAccumulator;

    DWORD pMeterBlinkTime;
    bool isPMeterBlinkVisible;

    void DrawString(std::string text, float x, float y);

public:
    HUD();
    static HUD* GetInstance();
    void LoadSprites();

    void Update(DWORD dt);
    void Render();

    void DrawCoins(int coins);
    void DrawScore(int score);
    void DrawTime(int time);
    void DrawPMeter(int powerLevel);
};