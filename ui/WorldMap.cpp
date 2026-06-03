#include "WorldMap.h"
#include "../render/Sprites.h"

WorldMap::WorldMap()
{
    isDone = false;
    selectedLevel = 1; 
    marioX = 100.0f;   
    marioY = 100.0f;
}

void WorldMap::LoadSprites()
{
}

void WorldMap::Update(DWORD dt)
{
    // 1. CHỐNG TRÔI PHÍM ENTER / SPACE
    static bool isEnterPressed = true;

    if ((GetAsyncKeyState(VK_RETURN) & 0x8000) || (GetAsyncKeyState(VK_SPACE) & 0x8000))
    {
        if (!isEnterPressed)
        {
            isDone = true;
            isEnterPressed = true;
        }
    }
    else
    {
        isEnterPressed = false;
    }

    // 2. CHỐNG TRÔI PHÍM MŨI TÊN
    static bool isRightPressed = false;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        if (!isRightPressed) {
            selectedLevel = 2;
            marioX = 200.0f;
            isRightPressed = true;
        }
    }
    else isRightPressed = false;

    static bool isLeftPressed = false;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
        if (!isLeftPressed) {
            selectedLevel = 1;
            marioX = 100.0f;
            isLeftPressed = true;
        }
    }
    else isLeftPressed = false;
}

void WorldMap::Render()
{
    Sprites* sprites = Sprites::GetInstance();

    // Vẽ 2 mốc lựa chọn màn chơi (Quay về nền đen cơ bản ban đầu)
    if (sprites->Get(10))
    {
        sprites->Get(10)->Draw(100.0f, 120.0f); // Mốc Level 1
        sprites->Get(10)->Draw(200.0f, 120.0f); // Mốc Level 2
    }

    // Vẽ Mario di chuyển trên mốc bản đồ
    if (sprites->Get(0))
    {
        sprites->Get(0)->Draw(marioX, marioY);
    }
}