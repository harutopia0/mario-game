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
    }
    return __instance;
}

void GameManager::ResetClearedLevels()
{
    for (int i = 0; i < 6; i++) clearedLevels[i] = false;
}

void GameManager::AddCard(int cardType)
{
    for (int i = 0; i < 3; i++)
    {
        if (holdingCards[i] == 0)
        {
            holdingCards[i] = cardType;
            break;
        }
    }
}

void GameManager::ClearHoldingCards()
{
    for (int i = 0; i < 3; i++) holdingCards[i] = 0;
}