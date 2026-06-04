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

void WorldMap::SetSelectedLevel(int level)
{
    selectedLevel = level;
    // Điều hướng tọa độ điểm dừng cho Icon Mario trên bản đồ
    if (selectedLevel == 1)
    {
        marioX = 100.0f;
        marioY = 100.0f;
    }
    else if (selectedLevel == 2)
    {
        marioX = 200.0f;
        marioY = 100.0f;
    }
    else if (selectedLevel == 3)
    {
        marioX = 300.0f;
        marioY = 100.0f;
    }
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

    // 2. DI CHUYỂN TUẦN TIẾN ĐỒNG BỘ 3 LEVEL
    static bool isRightPressed = false;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        if (!isRightPressed) {
            if (selectedLevel < 3) {
                SetSelectedLevel(selectedLevel + 1);
            }
            isRightPressed = true;
        }
    }
    else isRightPressed = false;

    static bool isLeftPressed = false;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
        if (!isLeftPressed) {
            if (selectedLevel > 1) {
                SetSelectedLevel(selectedLevel - 1);
            }
            isLeftPressed = true;
        }
    }
    else isLeftPressed = false;
}

void WorldMap::Render()
{
    Sprites* sprites = Sprites::GetInstance();

    // Vẽ 3 mốc điểm màn chơi
    if (sprites->Get(10))
    {
        sprites->Get(10)->Draw(100.0f, 120.0f);
        sprites->Get(10)->Draw(200.0f, 120.0f);
        sprites->Get(10)->Draw(300.0f, 120.0f);
    }

    if (sprites->Get(0))
    {
        sprites->Get(0)->Draw(marioX, marioY);
    }
}