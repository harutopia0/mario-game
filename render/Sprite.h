#pragma once
#include <d3d10.h>
#include <d3dx10.h>

class Sprite
{
private:
    int id;
    int left, top, right, bottom;
    ID3D10ShaderResourceView* texture;

    D3DX10_SPRITE spriteInfo;
    D3DXMATRIX matScaling;

public:
    Sprite(int id, int left, int top, int right, int bottom, ID3D10ShaderResourceView* tex, int texWidth, int texHeight);
    void Draw(float x, float y);
    ~Sprite();
};