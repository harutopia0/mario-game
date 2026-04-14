#pragma once
#include <unordered_map>
#include "Sprite.h"

class Sprites
{
private:
    static Sprites* __instance;
    std::unordered_map<int, Sprite*> sprites;

public:
    static Sprites* GetInstance();
    void Add(int id, int left, int top, int right, int bottom, ID3D10ShaderResourceView* tex, int texWidth, int texHeight);
    Sprite* Get(int id);
};