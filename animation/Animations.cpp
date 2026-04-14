#include "Animations.h"

Animations* Animations::__instance = NULL;

Animations* Animations::GetInstance()
{
    if (__instance == NULL) __instance = new Animations();
    return __instance;
}

void Animations::Add(int id, Animation* ani)
{
    animations[id] = ani;
}

Animation* Animations::Get(int id)
{
    return animations[id];
}