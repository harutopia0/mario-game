#pragma once
#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>

class Game
{
private:
    static Game* __instance;

    HWND hWnd;
    ID3D10Device* pDevice;
    IDXGISwapChain* pSwapChain;
    ID3D10RenderTargetView* pRenderTargetView;
    ID3DX10Sprite* pSpriteObject;
    ID3D10BlendState* pBlendStateAlpha;

public:
    static Game* GetInstance();
    void InitDirectX(HWND hWnd);
    bool LoadTexture(LPCWSTR filePath, ID3D10ShaderResourceView** outTexture);
    void ReleaseDirectX();
    ID3D10Device* GetDevice() { return pDevice; }
    IDXGISwapChain* GetSwapChain() { return pSwapChain; }
    ID3D10RenderTargetView* GetRenderTargetView() { return pRenderTargetView; }
    ID3DX10Sprite* GetSpriteHandler() { return pSpriteObject; }
    ID3D10BlendState* GetBlendState() { return pBlendStateAlpha; }

    ~Game();
};