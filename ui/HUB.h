#pragma once
#include <string>
#include <Windows.h>
#include "../render/Sprites.h"

class HUD
{
private:
    static HUD* __instance;
    int score;
    int coins;
    int time;

    void DrawNumber(int number, float x, float y);

public:
    HUD();
    static HUD* GetInstance();
    void Update(DWORD dt);
    void Render();

    void SetScore(int s) { score = s; }
    void AddScore(int s) { score += s; }

    void SetCoins(int c) { coins = c; }
    void AddCoin() { coins++; }

    void SetTime(int t) { time = t; }
};