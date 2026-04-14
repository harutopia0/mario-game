#pragma once
#include <unordered_map>
#include <d3d10.h>
#include <d3dx10.h>

class Textures
{
private:
    static Textures* __instance;
    std::unordered_map<int, ID3D10ShaderResourceView*> textures;

public:
    static Textures* GetInstance();
    void Add(int id, LPCWSTR filePath);
    ID3D10ShaderResourceView* Get(int id);
};