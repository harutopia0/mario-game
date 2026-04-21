#include "../render/Sprites.h"

Sprites* Sprites::__instance = NULL;

Sprites* Sprites::GetInstance()
{
    if (__instance == NULL) __instance = new Sprites();
    return __instance;
}

void Sprites::Add(int id, int left, int top, int right, int bottom, int textureId)
{
    LPDIRECT3DTEXTURE tex = Textures::GetInstance()->Get(textureId);

    Sprite* sprite = new Sprite(id, left, top, right, bottom, tex);
    sprites[id] = sprite;
}

Sprite* Sprites::Get(int id)
{
    return sprites[id];
}