#include "GameManager.h"

GameManager* GameManager::__instance = nullptr;

GameManager* GameManager::GetInstance()
{
    if (__instance == nullptr)
    {
        __instance = new GameManager();
    }
    return __instance;
}