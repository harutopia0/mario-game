#include "GameManager.h"

GameManager* GameManager::__instance = nullptr;

GameManager* GameManager::GetInstance()
{
    if (__instance == nullptr)
    {
        __instance = new GameManager();
        __instance->level = 1;
        __instance->isGameOver = false;
        __instance->isGameWin = false;
        __instance->isLevelClear = false;
        __instance->isDebugMode = false;
        for (int i = 0; i < 6; i++) __instance->clearedLevels[i] = false;
        for (int i = 0; i < 3; i++) __instance->holdingCards[i] = 0;

        __instance->score = 0;
        __instance->coins = 0;
        __instance->lives = 1;
        __instance->time = 300;
        __instance->timeAccumulator = 0;

        __instance->isMarioBig = false;
        __instance->isMarioFire = false;
        __instance->isMarioScissors = false;
    }
    return __instance;
}

void GameManager::DestroyInstance()
{
    if (__instance != nullptr)
    {
        delete __instance;
        __instance = nullptr;
    }
}

void GameManager::ResetClearedLevels()
{
    for (int i = 0; i < 6; i++) clearedLevels[i] = false;
}

bool GameManager::AddCard(int cardType)
{
    for (int i = 0; i < 3; i++)
    {
        if (holdingCards[i] == 0)
        {
            holdingCards[i] = cardType;
            return true;
        }
    }
    return false;
}

void GameManager::ClearHoldingCards()
{
    for (int i = 0; i < 3; i++) holdingCards[i] = 0;
}

int GameManager::UseCard(int slot)
{
    if (slot < 0 || slot > 2) return 0;
    int cardType = holdingCards[slot];
    holdingCards[slot] = 0;
    return cardType;
}

void GameManager::ResetTime()
{
    time = 300;
    timeAccumulator = 0;
}

void GameManager::UpdateTime(DWORD dt)
{
    if (isLevelClear || isGameWin) return;

    if (time > 0)
    {
        timeAccumulator += dt;
        if (timeAccumulator >= 1000)
        {
            time--;
            timeAccumulator = 0;
        }
    }
}


void GameManager::ResetForNewLevel()
{
    isGameOver = false;
    isGameWin = false;
    isLevelClear = false;
    ResetTime();
    // score, coins, lives, form Mario: giữ nguyên qua các màn
}