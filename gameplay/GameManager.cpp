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
    }
    return __instance;
}