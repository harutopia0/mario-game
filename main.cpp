#pragma region ImportLibaries

#include "core/Game.h"
#include "render/Sprites.h"
#include "animation/Animations.h"
#include "render/Textures.h"
#include "gameobject/Mario.h"
#include "gameobject/Brick.h"
#include "ui/HUB.h"
#include "ui/Intro.h"

#include <string.h>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <fstream>

#pragma endregion

#pragma region Settings

#define WINDOW_TITLE L"MarioX"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define FPS_LIMIT 100
#define GRID_CELL_SIZE 64.0f
#define MAX_CELL_ROW 50
#define MAX_CELL_COL 200

#pragma endregion

#pragma region GlobalVariables/GameObjects/ID_Definitions

std::vector<GameObject*> g_objectList;
std::vector<GameObject*> grid[MAX_CELL_ROW][MAX_CELL_COL];

bool g_showBBox = false;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING
};

GameState currentState = STATE_INTRO;

Intro* introScene = NULL;

enum TEXTURE_ID {
    TEX_MARIO = 0,
    TEX_COMMON = 1,
    TEX_FONT = 20,
    TEX_INTRO = 30,
    TEX_BBOX = 99
};

#pragma endregion

#pragma region FunctionPrototypes

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void LoadMap(LPCWSTR filePath);
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
        int soLanKiemTraVaCham = 0;
        for (GameObject* obj : g_objectList)
        {
            if (obj->isStatic == true) {
                obj->Update(dt, NULL);
                continue;
            }

            int currentCellX = (int)(obj->GetX() / GRID_CELL_SIZE);
            int currentCellY = (int)(obj->GetY() / GRID_CELL_SIZE);

            std::vector<GameObject*> nearbyObjects;

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int checkRow = currentCellY + i;
                    int checkCol = currentCellX + j;

                    if (checkRow >= 0 && checkRow < MAX_CELL_ROW && checkCol >= 0 && checkCol < MAX_CELL_COL) {
                        for (GameObject* g : grid[checkRow][checkCol]) {
                            nearbyObjects.push_back(g);
                        }
                    }
                }
            }
            soLanKiemTraVaCham += nearbyObjects.size();
            obj->Update(dt, &nearbyObjects);
        }
        char debugStr[100];
        sprintf_s(debugStr, "SO PHÉP TINH VA CHAM TRONG 1 FRAME: %d\n", soLanKiemTraVaCham);
        OutputDebugStringA(debugStr);
    }
}

// DRAWING (Show on screen)
void Render()
{
    Game* game = Game::GetInstance();
    ID3D10Device* dev = game->GetDevice();

    if (dev)
    {
        float bgColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        if (currentState == STATE_PLAYING) {
            bgColor[0] = 0.2f; bgColor[1] = 0.2f; bgColor[2] = 0.2f;
        }
        dev->ClearRenderTargetView(game->GetRenderTargetView(), bgColor);

        float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dev->OMSetBlendState(game->GetBlendState(), blendFactor, 0xffffffff);

        game->GetSpriteHandler()->Begin(D3DX10_SPRITE_SORT_TEXTURE);

        D3DXMATRIX matZoom;

        if (currentState == STATE_INTRO)
        {
            D3DXMatrixScaling(&matZoom, 1.0f, 1.0f, 1.0f);
            game->GetSpriteHandler()->SetViewTransform(&matZoom);

            if (introScene) introScene->Render();
        }
        else
        {
            D3DXMatrixScaling(&matZoom, 2.0f, 2.0f, 1.0f);
            game->GetSpriteHandler()->SetViewTransform(&matZoom);
            GameObject* mario = g_objectList.empty() ? NULL : g_objectList[0];

            for (GameObject* obj : g_objectList)
            {
                obj->Render();
                if (g_showBBox && mario != NULL)
                {
                    int marioCellX = (int)(mario->GetX() / GRID_CELL_SIZE);
                    int marioCellY = (int)(mario->GetY() / GRID_CELL_SIZE);

                    int objCellX = (int)(obj->GetX() / GRID_CELL_SIZE);
                    int objCellY = (int)(obj->GetY() / GRID_CELL_SIZE);

                    if (abs(marioCellX - objCellX) <= 1 && abs(marioCellY - objCellY) <= 1)
                    {
                        obj->RenderBoundingBox();
                    }
                }
            }
            D3DXMATRIX matUI;
            D3DXMatrixScaling(&matUI, 1.0f, 1.0f, 1.0f);
            game->GetSpriteHandler()->SetViewTransform(&matUI);

            HUD::GetInstance()->Render();
        }

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

void LoadMap(LPCWSTR filePath)
{
    ifstream f;
    f.open(filePath);

    if (!f.is_open()) return;

    int rows, cols;
    f >> rows >> cols;

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int tileID;
            f >> tileID;

            float realX = c * 16.0f;

            float realY = ((rows - r - 1) * 16.0f) + 50.0f;


            if (tileID == 1 || tileID == 2)
            {
                Brick* brick = new Brick(realX, realY);
                g_objectList.push_back(brick);

                int cellX = (int)(realX / GRID_CELL_SIZE);
                int cellY = (int)(realY / GRID_CELL_SIZE);

                if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 && cellY < MAX_CELL_ROW) {
                    grid[cellY][cellX].push_back(brick);
                }
            }
        }
    }
    f.close();
}

void LoadResources()
{
    Textures* textures = Textures::GetInstance();
    Sprites* sprites = Sprites::GetInstance();
    Animations* animations = Animations::GetInstance();
    Animation* ani;

    // ==========================================
    // 1. NẠP TÀI NGUYÊN
    // ==========================================

    textures->Add(TEX_MARIO, L"assets/mario.png");
    textures->Add(TEX_COMMON, L"assets/CommonObjects&Pipes.png");
    textures->Add(TEX_FONT, L"assets/font.png");
    textures->Add(TEX_INTRO, L"assets/intro_items.png");
    textures->Add(TEX_BBOX, L"assets/bbox.png");

    // ==========================================
    // 2. CẮT SPRITES
    // ==========================================

    // Idle
    sprites->Add(0, 211, 0, 223, 15, TEX_MARIO); // Phải
    sprites->Add(1, 181, 0, 194, 15, TEX_MARIO); // Trái
    
    // Run
    sprites->Add(2, 241, 0, 255, 14, TEX_MARIO);
    sprites->Add(3, 272, 0, 284, 15, TEX_MARIO);
    sprites->Add(4, 300, 0, 316, 15, TEX_MARIO);
    sprites->Add(5, 150, 0, 164, 14, TEX_MARIO);
    sprites->Add(6, 121, 0, 133, 15, TEX_MARIO);
    sprites->Add(7, 89, 0, 105, 15, TEX_MARIO);
    
    // Jump
    sprites->Add(8, 359, 0, 375, 15, TEX_MARIO);
    sprites->Add(9, 29, 0, 45, 15, TEX_MARIO);

    //  Brick
    sprites->Add(10, 119, 36, 134, 51, TEX_COMMON);

    // Intro
    sprites->Add(2000, 8, 19, 648, 499, TEX_INTRO);    // 2000: Nền màn nhung đỏ
    sprites->Add(2001, 8, 642, 308, 792, TEX_INTRO);  // 2001: Logo Super Mario
    sprites->Add(2002, 411, 601, 611, 651, TEX_INTRO); // 2002: Chữ 1 Player / 2 Player
    sprites->Add(2003, 388, 601, 409, 620, TEX_INTRO); // 2003: Con trỏ (Nấm)

    // Bounding Box
    sprites->Add(99999, 0, 0, 9, 9, 99);
    
    // ==========================================
    // 3. GOM SPRITES TẠO ANIMATION
    // ==========================================

    // Tạo animation cho Mario
    ani = new Animation(100); ani->Add(0, 1000); animations->Add(100, ani);
    ani = new Animation(100); ani->Add(1, 1000); animations->Add(101, ani);
    
    ani = new Animation(100); ani->Add(2); ani->Add(3); ani->Add(4); animations->Add(102, ani);
    ani = new Animation(100); ani->Add(5); ani->Add(6); ani->Add(7); animations->Add(103, ani);
    
    ani = new Animation(100); ani->Add(8, 1000); animations->Add(104, ani);
    ani = new Animation(100); ani->Add(9, 1000); animations->Add(105, ani);

    // Tạo animation cho Brick
    ani = new Animation(100); ani->Add(10, 1000); animations->Add(201, ani);

    // ==========================================
    // 4. KHỞI TẠO OBJECT
    // ==========================================

    // Khởi tạo Mario
    Mario* mario = new Mario(100.0f, 200.0f, MARIO_WIDTH, MARIO_HEIGHT);
    g_objectList.push_back(mario);

    // Khởi tạp map
    LoadMap(L"assets/testmap.txt");

    // Cắt 10 số (0-9)
    for (int i = 0; i < 10; i++)
    {
        sprites->Add(1000 + i, 22 + i * 16, 136, 22 + (i + 1) * 16, 136 + 16, TEX_FONT);
    }

    // Khởi tạo Intro
    introScene = new Intro();
}

void Cleanup()
{
    for (GameObject* obj : g_objectList) delete obj;
    g_objectList.clear();
    Game::GetInstance()->ReleaseDirectX();
    Animations::GetInstance()->Clear();
}

#pragma endregion