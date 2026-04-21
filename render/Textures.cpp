#include "Textures.h"
#include "../core/Game.h"

// --- PHẦN HÀM CỦA TEXTURES (SỐ NHIỀU) ---

Textures* Textures::__instance = NULL;

Textures::Textures() {} // Hàm khởi tạo rỗng

void Textures::Add(int id, LPCWSTR filePath)
{
    ID3D10ShaderResourceView* tex = NULL;
    D3DX10_IMAGE_INFO info;

    // 1. Tự động lấy kích thước file ảnh
    HRESULT hr = D3DX10GetImageInfoFromFile(filePath, NULL, &info, NULL);
    if (FAILED(hr))
    {
        OutputDebugString(L"[ERROR] Failed to get image info!\n");
        return;
    }

    // 2. Nạp ảnh vào Card đồ họa
    Game::GetInstance()->LoadTexture(filePath, &tex);

    // 3. Tạo 1 Texture (Số ít) mới và cất vào kho
    LPDIRECT3DTEXTURE texture = new Texture(tex, info.Width, info.Height);
    textures[id] = texture;
}

LPDIRECT3DTEXTURE Textures::Get(int id)
{
    return textures[id];
}

Textures* Textures::GetInstance()
{
    if (__instance == NULL) __instance = new Textures();
    return __instance;
}