#include "WorldMap.h"

WorldMap::WorldMap()
{
    isDone = false;
    selectedLevel = 1; // Mặc định ở Level 1
    marioX = 100.0f;   // Tọa độ giả định của Level 1
    marioY = 100.0f;
}

void WorldMap::LoadSprites()
{
    // Load ảnh nền map, icon Mario ở đây
}

void WorldMap::Update(DWORD dt)
{
    // 1. CHỐNG TRÔI PHÍM ENTER / SPACE
    // Khởi tạo true để nó bỏ qua cú nhấn Enter còn sót lại từ màn Intro
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
        // Khi người chơi thực sự nhả phím ra thì mới reset
        isEnterPressed = false;
    }


    // 2. CHỐNG TRÔI PHÍM MŨI TÊN (Tương tự)
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
    // Vẽ nền map
    // Sprites::GetInstance()->Get(ID_BG_MAP)->Draw(0, 0);

    // Vẽ icon Mario
    // Sprites::GetInstance()->Get(ID_MARIO_MAP)->Draw(marioX, marioY);
}