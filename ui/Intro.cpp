#include "Intro.h"

Intro::Intro()
{
    currentOption = 1;
    isDone = false;
}

void Intro::Update(DWORD dt)
{
    // Đổi lựa chọn bằng phím Lên/Xuống
    if (GetAsyncKeyState(VK_UP) & 0x8000)
    {
        currentOption = 1;
    }
    else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
    {
        currentOption = 2;
    }

    // Nhấn Enter hoặc Space để bắt đầu
    if ((GetAsyncKeyState(VK_RETURN) & 0x8000) || (GetAsyncKeyState(VK_SPACE) & 0x8000))
    {
        isDone = true;
    }
}

void Intro::Render()
{
    Sprites* sprites = Sprites::GetInstance();

    // ID giả định cho các sprite của Intro:
    // 2000: Nền (Màn nhung đỏ)
    // 2001: Logo Super Mario Bros 3
    // 2002: Chữ "1 PLAYER GAME" và "2 PLAYER GAME"
    // 2003: Con trỏ (Hình nấm hoặc mũi tên)

    if (sprites->Get(2000)) sprites->Get(2000)->Draw(0.0f, 0.0f); // Vẽ nền giữa màn hình
    if (sprites->Get(2001)) sprites->Get(2001)->Draw(170.0f, 275.0f); // Vẽ Logo phía trên
    if (sprites->Get(2002)) sprites->Get(2002)->Draw(220.0f, 125.0f); // Vẽ Text phía dưới

    // Vẽ con trỏ dựa theo lựa chọn hiện tại
    float cursorX = 185.0f;
    float cursorY = (currentOption == 1) ? 148.0f : 125.0f; // Toạ độ Y thay đổi theo dòng

    if (sprites->Get(2003)) sprites->Get(2003)->Draw(cursorX, cursorY);
}