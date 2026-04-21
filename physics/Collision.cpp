#include "Collision.h"
Collision* Collision::__instance = nullptr;

Collision* Collision::GetInstance()
{
	if (__instance == nullptr) __instance = new Collision();
	return __instance;
}