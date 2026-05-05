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

    int currentPMeter;
    int currentPlayer;
    int currentScore;
    int currentCoins;
    int currentLives;
    int currentWorld;

    int cards[3];

    void DrawString(std::string text, float x, float y);

public:
    static void DestroyInstance();

    HUD();
    static HUD* GetInstance();
    void LoadSprites();

    void Update(DWORD dt);
    void Render();

    void DrawCoins(int coins);
    void DrawScore(int score);
    void DrawTime(int time);
    void DrawPMeter(int powerLevel);
    void DrawPlayerIcon(int player);
    void DrawCards();
    void DrawLives(int lives);
    void DrawWorld(int world);

    void SetPMeter(int level) { currentPMeter = level; }
    void SetPlayer(int player) { currentPlayer = player; }
    void SetScore(int score) { currentScore = score; }
    void SetCoins(int coins) { currentCoins = coins; }
    void SetLives(int lives) { currentLives = lives; }
    void SetWorld(int world) { currentWorld = world; }

    void AddCard(int cardType);
    void ClearCards() {
        cards[0] = 0; cards[1] = 0; cards[2] = 0;
    }
};