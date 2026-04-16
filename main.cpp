#pragma region ImportLibaries

#include "core/Game.h"
#include "render/Sprites.h"
#include "animation/Animations.h"
#include "render/Textures.h"
#include "character/Mario.h"
#include "gameplay/Brick.h"
#include "Character/Mario.h"

#include <string.h>
#include <vector>
#include <stdlib.h>
#include <time.h>

// Thêm include ở đầu file main.cpp
#include "ui/HUB.h"
#include "ui/Intro.h"

// Khai báo trạng thái game
enum GameState {
    STATE_INTRO,
    STATE_PLAYING
};

GameState currentState = STATE_INTRO;
Intro* introScene = NULL;

#pragma endregion

#pragma region Settings

#define WINDOW_TITLE L"MarioX"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define FPS_LIMIT 100

#pragma endregion

#pragma region GlobalVariables_GameObjects

std::vector<GameObject*> g_objectList;
bool g_showBBox = false;

#pragma endregion

#pragma region FunctionPrototypes

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void LoadResources();
void Update(DWORD dt);
void Render();
void Cleanup();

#pragma endregion

#pragma region MainFunction

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    // 1. Create Window Class
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WinProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = L"GameWindowClass";
    RegisterClassEx(&wc);

    RECT wr = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    // 2. Create Window
    HWND hWnd = CreateWindow(
        L"GameWindowClass", WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL, NULL, hInstance, NULL);

    if (!hWnd) return 0;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 3. Initialize DirectX
    Game::GetInstance()->InitDirectX(hWnd);

    // 4. Load Images
    LoadResources();

    // 5. Game Loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    ULONGLONG frameStart = GetTickCount64();
    float tickPerFrame = 1000.0f / FPS_LIMIT;

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            ULONGLONG now = GetTickCount64();
            float dt = (float)(now - frameStart);

            if (dt >= tickPerFrame)
            {
                frameStart = now;

                // --- GAME LOGIC HERE ---
                Update(dt);
                Render();
            }
            else
            {
                Sleep(1); // Relax CPU
            }
        }
    }

    Cleanup();
    return 0;
}

#pragma endregion

#pragma region LogicSection

// CALCULATION (Physics, Movement)
void Update(DWORD dt)
{
    static bool isF2Pressed = false;
    if (GetAsyncKeyState(VK_F2) & 0x8000)
    {
        if (!isF2Pressed)
        {
            g_showBBox = !g_showBBox;
            isF2Pressed = true;
        }
    }
    else
    {
        isF2Pressed = false;
    }

    if (currentState == STATE_INTRO)
    {
        introScene->Update(dt);
        if (introScene->IsDone()) {
            currentState = STATE_PLAYING;
        }
    }
    else {
        HUD::GetInstance()->Update(dt);
        for (GameObject* obj : g_objectList) {
            obj->Update(dt, &g_objectList);
        }
    }
}

// DRAWING (Show on screen)
void Render()
{
    Game* game = Game::GetInstance();
    ID3D10Device* dev = game->GetDevice();

    if (dev)
    {
        // BƯỚC 1: Xóa màn hình (Lệnh này phải chạy đầu tiên)
        float bgColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Mặc định đen cho Intro
        if (currentState == STATE_PLAYING) {
            bgColor[0] = 0.2f; bgColor[1] = 0.2f; bgColor[2] = 0.2f; // Xám khi vào game
        }
        dev->ClearRenderTargetView(game->GetRenderTargetView(), bgColor);

        // BƯỚC 2: Thiết lập Blend State để vẽ được ảnh trong suốt (PNG)
        float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dev->OMSetBlendState(game->GetBlendState(), blendFactor, 0xffffffff);

        // BƯỚC 3: Bắt đầu vẽ Sprite
        game->GetSpriteHandler()->Begin(D3DX10_SPRITE_SORT_TEXTURE);

        if (currentState == STATE_INTRO)
        {
            // Vẽ các thành phần của Intro
            if (introScene) introScene->Render();
        }
        else
        {
            // Vẽ các đối tượng trong game (Mario, Quái...)
            for (GameObject* obj : g_objectList)
            {
                obj->Render();
                if (g_showBBox) obj->RenderBoundingBox();
            }
            // Vẽ HUD lên trên cùng
            HUD::GetInstance()->Render();
        }

        // BƯỚC 4: Kết thúc vẽ và đẩy lên màn hình
        game->GetSpriteHandler()->End();
        game->GetSwapChain()->Present(0, 0);
    }
}

#pragma endregion

#pragma region SetupSection

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void LoadResources()
{
    Textures* textures = Textures::GetInstance();
    Sprites* sprites = Sprites::GetInstance();
    Animations* animations = Animations::GetInstance();

    // ==========================================
    // 1. NẠP TÀI NGUYÊN CHO MARIO
    // ==========================================
    textures->Add(0, L"assets/mario.png");
    textures->Add(1, L"assets/CommonObjects&Pipes.png");
    textures->Add(99, L"assets/bbox.png");
    ID3D10ShaderResourceView* texMario = textures->Get(0);
    ID3D10ShaderResourceView* texCommon = textures->Get(1);
    ID3D10ShaderResourceView* texBBox = textures->Get(99);
    int marioTexWidth = 1215;
    int marioTexHeight = 564;

    int commonTexWidth = 1278;
    int commonTexHeight = 882;

    int bboxTexWidtth = 10;
    int bboxTexHeight = 10;

    // Idle
    sprites->Add(0, 628, 0, 676, 48, texMario, marioTexWidth, marioTexHeight); // Phải
    sprites->Add(1, 538, 0, 586, 48, texMario, marioTexWidth, marioTexHeight); // Trái
    
    // Run
    sprites->Add(2, 719, 0, 767, 48, texMario, marioTexWidth, marioTexHeight);
    sprites->Add(3, 809, 0, 857, 48, texMario, marioTexWidth, marioTexHeight);
    sprites->Add(4, 899, 0, 947, 48, texMario, marioTexWidth, marioTexHeight);
    sprites->Add(5, 446, 0, 494, 48, texMario, marioTexWidth, marioTexHeight);
    sprites->Add(6, 357, 0, 405, 48, texMario, marioTexWidth, marioTexHeight);
    sprites->Add(7, 267, 0, 315, 48, texMario, marioTexWidth, marioTexHeight);
    
    // Jump
    sprites->Add(8, 1077, 0, 1127, 50, texMario, marioTexWidth, marioTexHeight);
    sprites->Add(9, 87, 0, 137, 50, texMario, marioTexWidth, marioTexHeight);

    //Brick
    sprites->Add(10, 357, 108, 404, 155, texCommon, commonTexWidth, commonTexHeight);

    //Bounding Box
    sprites->Add(99999, 0, 0, 9, 9, texBBox, bboxTexWidtth, bboxTexHeight);

    Animation* ani;
    
    // Gom Animation
    ani = new Animation(100); ani->Add(0, 1000); animations->Add(100, ani);
    ani = new Animation(100); ani->Add(1, 1000); animations->Add(101, ani);
    
    ani = new Animation(100); ani->Add(2); ani->Add(3); ani->Add(4); animations->Add(102, ani);
    ani = new Animation(100); ani->Add(5); ani->Add(6); ani->Add(7); animations->Add(103, ani);
    
    ani = new Animation(100); ani->Add(8, 1000); animations->Add(104, ani);
    ani = new Animation(100); ani->Add(9, 1000); animations->Add(105, ani);

    //Brick
    ani = new Animation(100);
    ani->Add(10, 1000);
    animations->Add(201, ani);

    // Khởi tạo Mario
    Mario* mario = new Mario(WINDOW_WIDTH / 2.0f, 400.0f);
    g_objectList.push_back(mario);

    for (int i = 0; i < 14; i++)
    {
        Brick* brick = new Brick(i * 48.0f, 50.0f);
        g_objectList.push_back(brick);
    }

    // ==========================================
    // 2. NẠP TÀI NGUYÊN CHO HUD (SỐ TỪ 0-9)
    // ==========================================
    textures->Add(20, L"assets/font.png");
    ID3D10ShaderResourceView* texFont = textures->Get(20);
    
    // Vòng lặp cắt 10 số (0-9). 
    // Giả định: ảnh font.png có kích thước 160x16, mỗi số rộng 16px, cao 16px.
    for (int i = 0; i < 10; i++) 
    {
        sprites->Add(1000 + i, 22 + i * 16, 136, 22 + (i + 1) * 16, 136 + 16, texFont, 266, 159);
    }

    // ==========================================
    // 3. NẠP TÀI NGUYÊN CHO INTRO SCENE
    // ==========================================
    textures->Add(30, L"assets/intro_items.png");
    ID3D10ShaderResourceView* texIntro = textures->Get(30);
    
    // Giả định tọa độ cắt trên ảnh intro_items.png (kích thước gốc 1024x1024)
    sprites->Add(2000, 8, 19, 648, 499, texIntro, 658, 803);    // 2000: Nền màn nhung đỏ
    sprites->Add(2001, 8, 642, 308, 792, texIntro, 658, 803);  // 2001: Logo Super Mario
    sprites->Add(2002, 411, 601, 611, 651, texIntro, 658, 803); // 2002: Chữ 1 Player / 2 Player
    sprites->Add(2003, 388, 601, 409, 620, texIntro, 658, 803); // 2003: Con trỏ (Nấm)

    // Khởi tạo Intro
    introScene = new Intro();
}

void Cleanup()
{
    for (GameObject* obj : g_objectList) delete obj;
    g_objectList.clear();
    Game::GetInstance()->ReleaseDirectX();
}

#pragma endregion