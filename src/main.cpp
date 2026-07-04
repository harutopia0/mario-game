#pragma region ImportLibaries

#include "engine/audio/AudioManager.h"
#include "engine/core/Camera.h"
#include "engine/core/Game.h"
#include "engine/graphics/Animations.h"
#include "game/ResourceLoader.h"
#include "game/scenes/GameManager.h"
#include "game/scenes/Map.h"
#include "game/scenes/SceneManager.h"
#include "game/ui/HUD.h"

#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

#pragma endregion

#pragma region Settings

#define WINDOW_TITLE L"MarioX"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define FPS_LIMIT 100

#pragma endregion

#pragma region GlobalVariables/GameObjects/ID_Definitions

bool g_showBBox = false;

#pragma endregion

#pragma region FunctionPrototypes

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void Update(DWORD dt);
void Render();
void Cleanup();

#pragma endregion
#pragma region MainFunction

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
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

    RECT wr = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    // 2. Create Window
    HWND hWnd =
        CreateWindow(L"GameWindowClass", WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
                     CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left,
                     wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

    if (!hWnd)
        return 0;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 3. Initialize DirectX
    Game::GetInstance()->InitDirectX(hWnd);

    // 4. Load Images
    ResourceLoader::LoadAll();

    // 5. Game Loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    ULONGLONG frameStart = GetTickCount64();
    float tickPerFrame = 1000.0f / FPS_LIMIT;
    // 6. Initialize Camera
    // Vì Game đang dùng ma trận zoom 2.0f (Scale 2x) trong SceneManager,
    // nên chiều rộng thực tế của Camera trong thế giới game chỉ bằng một nửa
    // WINDOW_WIDTH
    Camera::GetInstance()->Init(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f,
                                1000.0f, 408.0f);

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

    // Dọn dẹp
    Cleanup();
    return 0;
}

#pragma endregion

#pragma region LogicSection

// CALCULATION (Physics, Movement)
void Update(DWORD dt)
{
    static bool isPaused = false;
    static bool isF5Pressed = false;
    if (GetAsyncKeyState(VK_F5) & 0x8000)
    {
        if (!isF5Pressed)
        {
            isPaused = !isPaused;
            if (isPaused)
            {
                AudioManager::GetInstance()->PauseMusic();
            }
            else
            {
                AudioManager::GetInstance()->ResumeMusic();
            }
            isF5Pressed = true;
        }
    }
    else
    {
        isF5Pressed = false;
    }

    if (isPaused)
    {
        return;
    }

    // F2: Toggle bounding box display
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

    // F3: Toggle debug mode (bỏ qua khoá màn chơi trên World Map)
    static bool isF3Pressed = false;
    if (GetAsyncKeyState(VK_F3) & 0x8000)
    {
        if (!isF3Pressed)
        {
            bool current = GameManager::GetInstance()->IsDebugMode();
            GameManager::GetInstance()->SetDebugMode(!current);
            isF3Pressed = true;
        }
    }
    else
    {
        isF3Pressed = false;
    }

    // F4: Debug - Chế độ thêm thẻ bài
    static bool isF4Pressed = false;
    static bool cardDebugMode = false;
    if (GetAsyncKeyState(VK_F4) & 0x8000)
    {
        if (!isF4Pressed)
        {
            cardDebugMode = !cardDebugMode;
            isF4Pressed = true;
        }
    }
    else
    {
        isF4Pressed = false;
    }

    // Xử lý thêm thẻ bài trong chế độ debug
    if (cardDebugMode)
    {
        GameManager *gm = GameManager::GetInstance();
        static bool isAPressed = false;
        static bool isSPressed = false;
        static bool isDPressed = false;
        static bool isFPressed = false;

        if (GetAsyncKeyState('A') & 0x8000)
        {
            if (!isAPressed)
            {
                gm->AddCard(1);
                isAPressed = true;
            }
        }
        else
            isAPressed = false;

        if (GetAsyncKeyState('S') & 0x8000)
        {
            if (!isSPressed)
            {
                gm->AddCard(2);
                isSPressed = true;
            }
        }
        else
            isSPressed = false;

        if (GetAsyncKeyState('D') & 0x8000)
        {
            if (!isDPressed)
            {
                gm->AddCard(3);
                isDPressed = true;
            }
        }
        else
            isDPressed = false;

        if (GetAsyncKeyState('F') & 0x8000)
        {
            if (!isFPressed)
            {
                gm->AddCard(4);
                isFPressed = true;
            }
        }
        else
            isFPressed = false;
    }

    // Removed redundant camera update here (now handled safely in SceneManager::Update)

    SceneManager::GetInstance()->Update(dt);
}

// DRAWING (Show on screen)
void Render()
{
    Game *game = Game::GetInstance();
    ID3D10Device *dev = game->GetDevice();

    if (dev)
    {
        float bgColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        if (SceneManager::GetInstance()->GetState() == STATE_PLAYING)
        {
            int currentLevel = GameManager::GetInstance()->GetLevel();

            if (currentLevel == 2 || currentLevel == 4 || currentLevel == 5)
            {
                // Đen (Lòng đất, Ban đêm, Lâu đài)
                bgColor[0] = 0.0f;
                bgColor[1] = 0.0f;
                bgColor[2] = 0.0f;
            }
            else
            {
                // Màu bầu trời (Layer 1: Xanh da trời trơn cho Đồng bằng và Mây)
                bgColor[0] = 92.0f / 255.0f;
                bgColor[1] = 148.0f / 255.0f;
                bgColor[2] = 252.0f / 255.0f;
            }
        }
        dev->ClearRenderTargetView(game->GetRenderTargetView(), bgColor);

        float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        dev->OMSetBlendState(game->GetBlendState(), blendFactor, 0xffffffff);

        game->GetSpriteHandler()->Begin(D3DX10_SPRITE_SORT_TEXTURE);
        D3DXMATRIX view = Camera::GetInstance()->GetViewMatrix();

        game->GetSpriteHandler()->SetViewTransform(&view);

        SceneManager::GetInstance()->Render();

        game->GetSpriteHandler()->End();
        game->GetSwapChain()->Present(0, 0);
    }
}

#pragma endregion

#pragma region SetupSection

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN: {
        bool isRepeat = (lParam & (1 << 30)) != 0;
        if (!isRepeat)
        {
            SceneManager::GetInstance()->OnKeyDown(wParam);
        }
    }
    break;
    case WM_KEYUP:
        SceneManager::GetInstance()->OnKeyUp(wParam);
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void Cleanup()
{
    Map::GetInstance()->Clear();

    // Dọn dẹp Scene Manager
    SceneManager::GetInstance()->Cleanup();

    HUD::DestroyInstance();
    Animations::GetInstance()->Clear();
    AudioManager::GetInstance()->CleanUp();

    Game::GetInstance()->ReleaseDirectX();
}

#pragma endregion
