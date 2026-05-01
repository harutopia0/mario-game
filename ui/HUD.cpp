#include "HUD.h"
#include <stdlib.h>

// Định nghĩa ID texture và tọa độ của các phần tử trên HUD
#define TEX_HUD 20
#define HUD_COINS_X 353.0f
#define HUD_COINS_Y 38.0f
#define HUD_SCORE_X 165.0f
#define HUD_SCORE_Y 19.0f
#define HUD_TIME_X 334.0f
#define HUD_TIME_Y 19.0f
#define HUD_PMETER_X 165.0f 
#define HUD_PMETER_Y 38.0f
#define HUD_ICON_X 52.0f 
#define HUD_ICON_Y 19.0f 

// Tọa độ X Y của số mạng (căn cạnh icon người chơi)
#define HUD_LIVES_X 111.0f
#define HUD_LIVES_Y 19.0f

// Tọa độ vẽ số World
#define HUD_WORLD_X 130.0f 
#define HUD_WORLD_Y 38.0f

// Tọa độ X Y của 3 thẻ bài trên HUD
#define HUD_CARD_1_X 426.0f
#define HUD_CARD_2_X 482.0f
#define HUD_CARD_3_X 538.0f
#define HUD_CARD_Y   14.0f

HUD* HUD::__instance = NULL;

HUD::HUD()
{
    // Khởi tạo thời gian và bộ đếm thời gian
    time = 300;
    timeAccumulator = 0;

    // Khởi tạo trạng thái nhấp nháy của PMeter
    pMeterBlinkTime = 0;
    isPMeterBlinkVisible = true;

    // Khởi tạo các giá trị mặc định cho HUD
    currentPMeter = 0;
    currentPlayer = 1;
    currentScore = 0;
    currentCoins = 0;
    currentLives = 4;
    currentWorld = 1;

    // Khởi tạo mảng thẻ bài với giá trị 0 (chưa có thẻ nào)
    cards[0] = 0;
    cards[1] = 0;
    cards[2] = 0;
}

HUD* HUD::GetInstance()
{
    if (__instance == NULL) __instance = new HUD();
    return __instance;
}

void HUD::LoadSprites()
{
    Sprites* sprites = Sprites::GetInstance();

    //Sprite nền HUD
    sprites->Add(3000, 0, 0, 640, 75, TEX_HUD);

    //Sprites số cho điểm, tiền, thời gian (1000-1009)
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

    //Sprites mũi tên PMeter
    sprites->Add(3010, 932, 76, 951, 92, TEX_HUD);
    sprites->Add(3011, 953, 76, 988, 92, TEX_HUD);

    //Sprites icon người chơi
    sprites->Add(3012, 601, 86, 639, 102, TEX_HUD);
    sprites->Add(3013, 601, 103, 639, 119, TEX_HUD);

    //Sprites thẻ bài
    sprites->Add(3014, 1025, 83, 1068, 130, TEX_HUD); // 3014: Nấm
    sprites->Add(3015, 1081, 83, 1124, 130, TEX_HUD); // 3015: Hoa
    sprites->Add(3016, 1137, 83, 1180, 130, TEX_HUD); // 3016: Sao
}

void HUD::Update(DWORD dt)
{
    if (time > 0)
    {
        timeAccumulator += dt;
        if (timeAccumulator >= 1000)
        {
            time--;
            timeAccumulator = 0;
        }
    }

    // ==========================================
    // CODE TEST: ẤN 'R' ĐỂ GIẢM SỐ MẠNG
    // ==========================================
    static bool isRPressed = false;
    if (GetAsyncKeyState('R') & 0x8000)
    {
        if (!isRPressed)
        {
            currentLives--;
            if (currentLives < 0) currentLives = 99; // Nếu dưới 0 thì quay lại 99
            isRPressed = true;
        }
    }
    else
    {
        isRPressed = false;
    }

    pMeterBlinkTime += dt;
    if (pMeterBlinkTime >= 150)
    {
        isPMeterBlinkVisible = !isPMeterBlinkVisible;
        pMeterBlinkTime = 0;
    }

    // ==========================================
    // CODE TEST: TỰ ĐỘNG THÊM THẺ BÀI RANDOM MỖI 1 GIÂY
    // ==========================================
    static DWORD testCardTimer = 0;
    static int testCardCount = 0;

    testCardTimer += dt;
    if (testCardTimer >= 1000)
    {
        testCardCount++;

        if (testCardCount <= 3)
        {
            int randomItem = (rand() % 3) + 1;
            AddCard(randomItem);
        }
        else
        {
            ClearCards();
            testCardCount = 0;
        }

        testCardTimer = 0;
    }
}

void HUD::Render()
{
    Sprites* sprites = Sprites::GetInstance();

    if (sprites->Get(3000)) sprites->Get(3000)->Draw(0.0f, 0.0f);

    // Vẽ các phần tử HUD theo thứ tự
    DrawScore(currentScore);
    DrawCoins(currentCoins);
    DrawTime(time);
    DrawPMeter(currentPMeter);
    DrawPlayerIcon(currentPlayer);
    DrawLives(currentLives);
    DrawWorld(currentWorld);
    DrawCards();
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

void HUD::DrawLives(int lives)
{
    std::string str = std::to_string(lives);
    while (str.length() < 2) str = "0" + str; // Đảm bảo luôn có 2 số (VD: 04)

    DrawString(str, HUD_LIVES_X, HUD_LIVES_Y);
}

void HUD::DrawWorld(int world)
{
    // Chỉ cần 1 chữ số cho World
    std::string str = std::to_string(world);
    DrawString(str, HUD_WORLD_X, HUD_WORLD_Y);
}

void HUD::DrawPMeter(int powerLevel)
{
    Sprites* sprites = Sprites::GetInstance();
    float currentX = HUD_PMETER_X;
    float arrowWidth = 19.0f;

    for (int i = 0; i < 6; i++)
    {
        if (powerLevel > i)
        {
            if (sprites->Get(3010)) sprites->Get(3010)->Draw(currentX, HUD_PMETER_Y);
        }
        currentX += arrowWidth;
    }

    currentX += 1.0f;

    if (powerLevel >= 7)
    {
        if (isPMeterBlinkVisible)
        {
            if (sprites->Get(3011)) sprites->Get(3011)->Draw(currentX, HUD_PMETER_Y);
        }
    }
}

void HUD::DrawPlayerIcon(int player)
{
    Sprites* sprites = Sprites::GetInstance();

    if (player == 1)
    {
        if (sprites->Get(3012)) sprites->Get(3012)->Draw(HUD_ICON_X, HUD_ICON_Y);
    }
    else if (player == 2)
    {
        if (sprites->Get(3013)) sprites->Get(3013)->Draw(HUD_ICON_X, HUD_ICON_Y);
    }
}

void HUD::DrawCards()
{
    Sprites* sprites = Sprites::GetInstance();
    float cardX[3] = { HUD_CARD_1_X, HUD_CARD_2_X, HUD_CARD_3_X };

    for (int i = 0; i < 3; i++)
    {
        int spriteId = 0;

        if (cards[i] == 1) spriteId = 3014;
        else if (cards[i] == 2) spriteId = 3015;
        else if (cards[i] == 3) spriteId = 3016;

        if (spriteId != 0 && sprites->Get(spriteId))
        {
            sprites->Get(spriteId)->Draw(cardX[i], HUD_CARD_Y);
        }
    }
}

void HUD::AddCard(int cardType)
{
    for (int i = 0; i < 3; i++)
    {
        if (cards[i] == 0)
        {
            cards[i] = cardType;
            break;
        }
    }
}

void HUD::DestroyInstance()
{
    if (__instance != NULL)
    {
        delete __instance;
        __instance = NULL;
    }
}