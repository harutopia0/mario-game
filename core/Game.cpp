#include "Game.h"

Game* Game::__instance = NULL;

Game* Game::GetInstance()
{
    if (__instance == NULL) __instance = new Game();
    return __instance;
}

void Game::InitDirectX(HWND hWnd)
{
    this->hWnd = hWnd;

    RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;

    D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, D3D10_SDK_VERSION, &sd, &pSwapChain, &pDevice);

    ID3D10Texture2D* pBackBuffer = NULL;
    if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer)))
    {
        pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
        pBackBuffer->Release();
    }
    pDevice->OMSetRenderTargets(1, &pRenderTargetView, NULL);

    D3D10_VIEWPORT vp;
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    pDevice->RSSetViewports(1, &vp);

    D3DX10CreateSprite(pDevice, 0, &pSpriteObject);

    D3DXMATRIX matProjection;
    D3DXMatrixOrthoOffCenterLH(&matProjection, 0, (float)width, 0, (float)height, 0.1f, 10.0f);
    pSpriteObject->SetProjectionTransform(&matProjection);

    D3D10_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.BlendEnable[0] = TRUE;
    blendDesc.SrcBlend = D3D10_BLEND_SRC_ALPHA;
    blendDesc.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
    blendDesc.BlendOp = D3D10_BLEND_OP_ADD;
    blendDesc.SrcBlendAlpha = D3D10_BLEND_ONE;
    blendDesc.DestBlendAlpha = D3D10_BLEND_ZERO;
    blendDesc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
    blendDesc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;

    pDevice->CreateBlendState(&blendDesc, &pBlendStateAlpha);
}

bool Game::LoadTexture(LPCWSTR filePath, ID3D10ShaderResourceView** outTexture)
{
    HRESULT hr = D3DX10CreateShaderResourceViewFromFile(
        pDevice,
        filePath,
        NULL,
        NULL,
        outTexture,
        NULL
    );

    if (FAILED(hr))
    {
        OutputDebugString(L"[ERROR] Failed to load texture!\n");
        return false;
    }

    return true;
}

void Game::ReleaseDirectX()
{
    if (pRenderTargetView) pRenderTargetView->Release();
    if (pSwapChain) pSwapChain->Release();
    if (pDevice) pDevice->Release();
    if (pSpriteObject) pSpriteObject->Release();
    if (pBlendStateAlpha) pBlendStateAlpha->Release();
}

Game::~Game() {}