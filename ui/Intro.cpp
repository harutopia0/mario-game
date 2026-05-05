#include "Intro.h"
#include "HUD.h"

#define TEX_INTRO 30




void Intro::LoadSprites()
{
    Sprites* sprites = Sprites::GetInstance();

    sprites->Add(2000, 8, 19, 648, 434, TEX_INTRO);    // 2000: Nền màn nhung đỏ
    sprites->Add(2004, 8, 434, 648, 499, TEX_INTRO);   // 2004: Sàn caro
    sprites->Add(2001, 653, 19, 1293, 312, TEX_INTRO); // 2001: Logo Super Mario
    sprites->Add(2002, 411, 601, 611, 651, TEX_INTRO); // 2002: Chữ 1 Player / 2 Player
    sprites->Add(2003, 388, 601, 409, 620, TEX_INTRO); // 2003: Con trỏ (Nấm)
    sprites->Add(2005, 1300, 19, 1940, 434, TEX_INTRO); // 2005: Nền màu mới

    // THÊM: 4 Frame màu của số 3 (Sửa lại toạ độ cắt ảnh cho đúng)
    sprites->Add(2006, 771, 330, 876, 421, TEX_INTRO); // Số 3 - Màu 1
    sprites->Add(2007, 771, 422, 876, 513, TEX_INTRO); // Số 3 - Màu 2
    sprites->Add(2008, 877, 330, 982, 421, TEX_INTRO); // Số 3 - Màu 3
    sprites->Add(2009, 877, 422, 982, 513, TEX_INTRO); // Số 3 - Màu 4
    sprites->Add(2010, 983, 330, 1088, 421, TEX_INTRO); // Số 3 - Màu 5
    sprites->Add(2011, 983, 422, 1088, 513, TEX_INTRO); // Số 3 - Màu 6
}

Intro::Intro()
{
    currentOption = 1;
    isDone = false;

    curtainY = 60.0f;
    curtainSpeed = 0.2f;
    isCurtainUp = false;
    curtainHeight = 500.0f;

    logoY = 500.0f;
    logoSpeed = 0.2f;
    isLogoDown = false;

    // Khởi tạo biến cho số 3
    timerNum3 = 0;
    stateNum3 = 0;
}

void Intro::Update(DWORD dt)
{
    if (!isCurtainUp)
    {
        curtainY += curtainSpeed * dt;

        if (abs(curtainY) > curtainHeight)
        {
            isCurtainUp = true;
        }
    }
    else if (!isLogoDown)
    {
        logoY -= logoSpeed * dt;

        if (logoY <= 175.0f)
        {
            logoY = 175.0f;
            isLogoDown = true;
        }
    }
    else
    {
        // 1. Cập nhật hiệu ứng đổi màu số 3 (mỗi 300ms)
        timerNum3 += dt;
        if (timerNum3 > 300) // Bạn có thể tăng/giảm 100 để đổi tốc độ nhấp nháy
        {
            stateNum3++;
            if (stateNum3 > 5) stateNum3 = 0; // Trở về màu đầu tiên (có 6 màu: 0, 1, 2, 3, 4, 5)
            timerNum3 = 0;
        }

        // 2. Xử lý thao tác menu
        if (GetAsyncKeyState(VK_UP) & 0x8000)
        {
            currentOption = 1;
			HUD::GetInstance()->SetPlayer(1); // Cập nhật lựa chọn Player 1 cho HUD
        }
        else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
        {
            currentOption = 2;
			HUD::GetInstance()->SetPlayer(2); // Cập nhật lựa chọn Player 2 cho HUD
        }

        if ((GetAsyncKeyState(VK_RETURN) & 0x8000) || (GetAsyncKeyState(VK_SPACE) & 0x8000))
        {
            isDone = true;
        }
    }
}

void Intro::Render()
{
    Sprites* sprites = Sprites::GetInstance();

    // NẾU LOGO ĐÃ HẠ XUỐNG XONG
    if (isLogoDown)
    {
        // Vẽ nền màu mới và sàn
        if (sprites->Get(2005)) sprites->Get(2005)->Draw(0.0f, 60.0f);
        if (sprites->Get(2004)) sprites->Get(2004)->Draw(0.0f, 0.0f);

        // Vẽ số 3 nhấp nháy đè lên (ID từ 2006 đến 2011)
        int currentNum3SpriteID = 2006 + stateNum3;
        if (sprites->Get(currentNum3SpriteID)) sprites->Get(currentNum3SpriteID)->Draw(280.0f, 172.0f);

        // Hiện Text và Con trỏ
        if (sprites->Get(2002)) sprites->Get(2002)->Draw(220.0f, 100.0f);
        float cursorX = 185.0f;
        float cursorY = (currentOption == 1) ? 132.0f : 98.0f;
        if (sprites->Get(2003)) sprites->Get(2003)->Draw(cursorX, cursorY);
    }
    // NẾU LOGO CHƯA XUỐNG XONG HOẶC MÀN NHUNG ĐANG KÉO
    else
    {
        if (sprites->Get(2004)) sprites->Get(2004)->Draw(0.0f, 0.0f);
        if (sprites->Get(2001)) sprites->Get(2001)->Draw(0.0f, logoY);

        if (!isCurtainUp)
        {
            if (sprites->Get(2000)) sprites->Get(2000)->Draw(0.0f, curtainY);
        }
    }
}