#pragma once
#include <d3d10.h>
#include <d3dx10.h>
#include "Textures.h"

class Sprite
{
private:
    int id;
    int left, top, right, bottom;
    ID3D10ShaderResourceView* texture;

    D3DX10_SPRITE spriteInfo;
    D3DXMATRIX matScaling;

public:
    Sprite(int id, int left, int top, int right, int bottom, LPDIRECT3DTEXTURE tex);
    void Draw(float x, float y);
    void Draw(float x, float y, float drawWidth, float drawHeight, float alpha = 0.5f);
    ~Sprite();
};