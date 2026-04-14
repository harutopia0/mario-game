#include "Textures.h"
#include "../core/Game.h"

Textures* Textures::__instance = NULL;

Textures* Textures::GetInstance()
{
    if (__instance == NULL) __instance = new Textures();
    return __instance;
}

void Textures::Add(int id, LPCWSTR filePath)
{
    ID3D10ShaderResourceView* tex = NULL;

    Game::GetInstance()->LoadTexture(filePath, &tex);

    textures[id] = tex;
}

ID3D10ShaderResourceView* Textures::Get(int id)
{
    return textures[id];
}