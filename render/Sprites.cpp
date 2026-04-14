#include "../render/Sprites.h"

Sprites* Sprites::__instance = NULL;

Sprites* Sprites::GetInstance()
{
    if (__instance == NULL) __instance = new Sprites();
    return __instance;
}

void Sprites::Add(int id, int left, int top, int right, int bottom, ID3D10ShaderResourceView* tex, int texWidth, int texHeight)
{
    Sprite* sprite = new Sprite(id, left, top, right, bottom, tex, texWidth, texHeight);
    sprites[id] = sprite;
}

Sprite* Sprites::Get(int id)
{
    return sprites[id];
}