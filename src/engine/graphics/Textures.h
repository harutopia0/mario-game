#pragma once
#include <unordered_map>
#include <d3d10.h>
#include <d3dx10.h>

using namespace std;

// ==========================================
// 1. LỚP SỐ ÍT: Quản lý 1 bức ảnh duy nhất
// ==========================================
class Texture
{
    ID3D10ShaderResourceView* shaderResourceView;
    int width;
    int height;

public:
    Texture(ID3D10ShaderResourceView* srv, int w, int h) {
        shaderResourceView = srv;
        width = w;
        height = h;
    }
    ~Texture() { if (shaderResourceView) shaderResourceView->Release(); }

    ID3D10ShaderResourceView* GetShaderResourceView() { return shaderResourceView; }
    int GetWidth() { return width; }
    int GetHeight() { return height; }
};

typedef Texture* LPDIRECT3DTEXTURE;

// ==========================================
// 2. LỚP SỐ NHIỀU: Quản lý kho chứa ảnh
// ==========================================
class Textures
{
    static Textures* __instance;
    unordered_map<int, LPDIRECT3DTEXTURE> textures;

public:
    Textures();
    void Add(int id, LPCWSTR filePath);
    LPDIRECT3DTEXTURE Get(int id);

    static Textures* GetInstance();
};