#include "HUD.h"

#define TEX_HUD 20
#define HUD_COINS_X 353.0f
#define HUD_COINS_Y 38.0f
#define HUD_SCORE_X 165.0f
#define HUD_SCORE_Y 19.0f
#define HUD_TIME_X 334.0f
#define HUD_TIME_Y 19.0f
#define HUD_PMETER_X 165.0f 
#define HUD_PMETER_Y 38.0f

HUD* HUD::__instance = NULL;

HUD::HUD()
{
    time = 300;
    timeAccumulator = 0;

    pMeterBlinkTime = 0;
    isPMeterBlinkVisible = true;

    currentPMeter = 0;
}

HUD* HUD::GetInstance()
{
    if (__instance == NULL) __instance = new HUD();
    return __instance;
}

void HUD::LoadSprites()
{
    Sprites* sprites = Sprites::GetInstance();

    sprites->Add(3000, 0, 0, 640, 75, TEX_HUD); // Nền HUD trống

    sprites->Add(1000, 649, 75, 668, 91, TEX_HUD);
    sprites->Add(1001, 669, 75, 688, 91, TEX_HUD);
    sprites->Add(1002, 689, 75, 708, 91, TEX_HUD);
    sprites->Add(1003, 709, 75, 728, 91, TEX_HUD);
    sprites->Add(1004, 729, 75, 748, 91, TEX_HUD);
    sprites->Add(1005, 749, 75, 768, 91, TEX_HUD);
    sprites->Add(1006, 769, 75, 788, 91, TEX_HUD);
    sprites->Add(1007, 789, 75, 808, 91, TEX_HUD);
    sprites->Add(1008, 809, 75, 828, 91, TEX_HUD);
    sprites->Add(1009, 829, 75, 848, 91, TEX_HUD);

    sprites->Add(3010, 932, 76, 951, 92, TEX_HUD); // Mũi tên sáng
    sprites->Add(3011, 953, 76, 988, 92, TEX_HUD); // Chữ P sáng
}

void HUD::Update(DWORD dt)
{
    // 1. Cập nhật đếm ngược thời gian
    if (time > 0)
    {
        timeAccumulator += dt;
        if (timeAccumulator >= 1000)
        {
            time--;
            timeAccumulator = 0;
        }
    }

    // 2. Cập nhật nhấp nháy chữ P (đổi trạng thái mỗi 150ms)
    pMeterBlinkTime += dt;
    if (pMeterBlinkTime >= 150)
    {
        isPMeterBlinkVisible = !isPMeterBlinkVisible;
        pMeterBlinkTime = 0;
    }
}

void HUD::Render()
{
    Sprites* sprites = Sprites::GetInstance();

    if (sprites->Get(3000)) sprites->Get(3000)->Draw(0.0f, 0.0f);

    DrawScore(0);
    DrawCoins(0);
    DrawTime(time);

    DrawPMeter(currentPMeter);

}

void HUD::DrawString(std::string text, float x, float y)
{
    float currentX = x;
    float charWidth = 19.0f;

    for (char c : text)
    {
        if (c >= '0' && c <= '9')
        {
            int spriteId = 1000 + (c - '0');
            Sprite* sprite = Sprites::GetInstance()->Get(spriteId);

            if (sprite)
            {
                sprite->Draw(currentX, y);
            }
        }
        currentX += charWidth;
    }
}

void HUD::DrawCoins(int coins)
{
    std::string str = std::to_string(coins);
    while (str.length() < 2) str = "0" + str;

    DrawString(str, HUD_COINS_X, HUD_COINS_Y);
}

void HUD::DrawScore(int score)
{
    std::string str = std::to_string(score);
    while (str.length() < 7) str = "0" + str;

    DrawString(str, HUD_SCORE_X, HUD_SCORE_Y);
}

void HUD::DrawTime(int t)
{
    std::string str = std::to_string(t);
    while (str.length() < 3) str = "0" + str;

    DrawString(str, HUD_TIME_X, HUD_TIME_Y);
}

void HUD::DrawPMeter(int powerLevel)
{
    Sprites* sprites = Sprites::GetInstance();
    float currentX = HUD_PMETER_X;
    float arrowWidth = 19.0f; // Kích thước mũi tên mới

    // 1. Vẽ 6 mũi tên
    for (int i = 0; i < 6; i++)
    {
        if (powerLevel > i)
        {
            if (sprites->Get(3010)) sprites->Get(3010)->Draw(currentX, HUD_PMETER_Y);
        }
        currentX += arrowWidth;
    }

    // 2. Vẽ chữ P
	currentX += 1.0f;  // chữ P cách mũi tên 1px để đẹp hơn


    if (powerLevel >= 7)
    {
        // Nhấp nháy: Chỉ vẽ đè sprite chữ P màu trắng khi trạng thái bật
        if (isPMeterBlinkVisible)
        {
            if (sprites->Get(3011)) sprites->Get(3011)->Draw(currentX, HUD_PMETER_Y);
        }
    }
}