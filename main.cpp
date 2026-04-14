/* =============================================================
   SE102 - MARIOX
   ============================================================= */

 // --- IMPORT LIBARIES ---
#pragma region ImportLibaries

#include "core/Game.h"
#include "render/Sprites.h"
#include "animation/Animations.h"
#include "render/Textures.h"

#include "character/Mario.h"

#include <string.h>
#include <vector>
#include <stdlib.h>
#include <time.h>

#pragma endregion


// --- SETTINGS ---
#pragma region Settings

#define WINDOW_TITLE L"MarioX"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define FPS_LIMIT 100

#pragma endregion


// --- GLOBAL VARIABLES (GAME OBJECTS) ---
#pragma region GlobalVariables_GameObjects

std::vector<GameObject*> g_objectList;

#pragma endregion


// --- FUNCTION PROTOTYPES ---
#pragma region FunctionPrototypes

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void LoadResources();
void Update(DWORD dt);
void Render();
void Cleanup();

#pragma endregion


// =============================================================
// MAIN FUNCTION
// =============================================================
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


// =============================================================
// LOGIC SECTION
// =============================================================
#pragma region LogicSection

// CALCULATION (Physics, Movement)
void Update(DWORD dt)
{
    for (GameObject* obj : g_objectList)
    {
        obj->Update((DWORD)dt);
    }
}

// DRAWING (Show on screen)
void Render()
{
    Game* game = Game::GetInstance();
    ID3D10Device* dev = game->GetDevice();

    if (dev)
    {
        float bgColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
        dev->ClearRenderTargetView(game->GetRenderTargetView(), bgColor);

        float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dev->OMSetBlendState(game->GetBlendState(), blendFactor, 0xffffffff);

        game->GetSpriteHandler()->Begin(D3DX10_SPRITE_SORT_TEXTURE);

        for (GameObject* obj : g_objectList)
        {
            obj->Render();
        }

        game->GetSpriteHandler()->End();
        game->GetSwapChain()->Present(0, 0);
    }
}

#pragma endregion


// =============================================================
// SETUP SECTION
// =============================================================
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
    //1. Tải Textures
    Textures* textures = Textures::GetInstance();

    textures->Add(0, L"assets/mario.png");

    ID3D10ShaderResourceView* texMario = textures->Get(0);

    int marioTexWidth = 1215;
    int marioTexHeight = 564;

    //2. Cắt thành Sprites
    Sprites* sprites = Sprites::GetInstance();

    //Idle Right
    sprites->Add(0, 628, 0, 676, 48, texMario, marioTexWidth, marioTexHeight);
    //Idle Left
    sprites->Add(1, 538, 0, 586, 48, texMario, marioTexWidth, marioTexHeight);

    //Moving Right
    sprites->Add(2, 719, 0, 767, 48, texMario, marioTexWidth, marioTexHeight);
    sprites->Add(3, 809, 0, 857, 48, texMario, marioTexWidth, marioTexHeight);
    sprites->Add(4, 899, 0, 947, 48, texMario, marioTexWidth, marioTexHeight);
    //Moving Left
    sprites->Add(5, 446, 0, 494, 48, texMario, marioTexWidth, marioTexHeight);
    sprites->Add(6, 357, 0, 405, 48, texMario, marioTexWidth, marioTexHeight);
    sprites->Add(7, 267, 0, 315, 48, texMario, marioTexWidth, marioTexHeight);

    //Jumping Right
    sprites->Add(8, 1077, 0, 1127, 50, texMario, marioTexWidth, marioTexHeight);
    //Jumping Left
    sprites->Add(9, 87, 0, 137, 50, texMario, marioTexWidth, marioTexHeight);

    //3. Gom thành Animation
    Animations* animations = Animations::GetInstance();
    Animation* ani;

    //Idle Right
    ani = new Animation(100);
    ani->Add(0, 1000);
    animations->Add(100, ani);
    //Idle Left
    ani = new Animation(100);
    ani->Add(1, 1000);
    animations->Add(101, ani);

    //Moving Right
    ani = new Animation(100);
    ani->Add(2); ani->Add(3); ani->Add(4);
    animations->Add(102, ani);

    //Moving Left
    ani = new Animation(100);
    ani->Add(5); ani->Add(6); ani->Add(7);
    animations->Add(103, ani);
    
    //Jumping Right
    ani = new Animation(100);
    ani->Add(8, 1000);
    animations->Add(104, ani);
    //Jumping Left
    ani = new Animation(100);
    ani->Add(9, 1000);
    animations->Add(105, ani);

    //4. Khởi tạo
    Mario* mario = new Mario(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
    g_objectList.push_back(mario);
}

void Cleanup()
{
    for (GameObject* obj : g_objectList) delete obj;
    g_objectList.clear();
    Game::GetInstance()->ReleaseDirectX();
}

#pragma endregion