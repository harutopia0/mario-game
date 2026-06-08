#pragma region ImportLibaries

#include "core/Game.h"
#include "render/Sprites.h"
#include "animation/Animations.h"
#include "render/Textures.h"
#include "gameobject/Mario.h"
#include "gameobject/Breakable.h"
#include "gameobject/Brick.h"
#include "gameobject/Buff.h"
#include "gameobject/Enemy.h"
#include "gameobject/Flag.h"
#include "gameobject/LuckyBlock.h"
#include "gameobject/Pipe.h"
#include "gameobject/Platform.h"
#include "ui/HUD.h"
#include "audio/AudioManager.h"
#include "gameplay/GameManager.h"
#include "gameplay/SceneManager.h"

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

#pragma endregion

#pragma region GlobalVariables/GameObjects/ID_Definitions

std::vector<GameObject*> g_objectList;
std::vector<GameObject*> grid[MAX_CELL_ROW][MAX_CELL_COL];

bool g_showBBox = false;

enum TEXTURE_ID {
    TEX_MARIO = 0,
    TEX_COMMON1 = 1,
    TEX_COMMON2 = 2,
    TEX_HUD = 20,
    TEX_INTRO = 30,
    TEX_BBOX = 99,
    TEX_ENEMY_TEST = 100,
    TEX_POTION = 101,
    TEX_FLAG = 102,
    TEX_LEVEL_CLEAR = 701,
    TEX_GAME_OVER = 702,
    TEX_YOU_WIN = 703,
	TEX_MAP_LEVEL = 800
};

#pragma endregion

#pragma region FunctionPrototypes

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void LoadMap(LPCWSTR filePath);
void LoadResources();
void Update(DWORD dt);
void Render();
void Cleanup();
void AddObjectToGrid(GameObject* obj);
void RemoveObjectFromGrid(GameObject* obj);
void UpdateObjectGrid(GameObject* obj);
void SpawnEnemy(float x, float y);

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

    SceneManager::GetInstance()->Update(dt);
}

// DRAWING (Show on screen)
void Render()
{
    Game* game = Game::GetInstance();
    ID3D10Device* dev = game->GetDevice();

    if (dev)
    {
        float bgColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        if (SceneManager::GetInstance()->GetState() == STATE_PLAYING) {
            bgColor[0] = 0.2f; bgColor[1] = 0.2f; bgColor[2] = 0.2f;
        }
        dev->ClearRenderTargetView(game->GetRenderTargetView(), bgColor);

        float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dev->OMSetBlendState(game->GetBlendState(), blendFactor, 0xffffffff);

        game->GetSpriteHandler()->Begin(D3DX10_SPRITE_SORT_TEXTURE);

        SceneManager::GetInstance()->Render();

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

            float realX = c * 15.0f;
            float realY = ((rows - r - 1) * 15.0f) + 50.0f;

            if (tileID == 1 || tileID == 2)
            {
                Brick* brick = new Brick(realX, realY, 201);
                g_objectList.push_back(brick);

                int cellX = (int)(realX / GRID_CELL_SIZE);
                int cellY = (int)(realY / GRID_CELL_SIZE);

                if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 && cellY < MAX_CELL_ROW) {
                    AddObjectToGrid(brick);
                }
            }
            else if (tileID == 3)
            {
                Platform* platform = new Platform(realX, realY, 202);
                g_objectList.push_back(platform);

                int cellX = (int)(realX / GRID_CELL_SIZE);
                int cellY = (int)(realY / GRID_CELL_SIZE);

                if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 && cellY < MAX_CELL_ROW) {
                    AddObjectToGrid(platform);
                }
            }
            else if (tileID == 4)
            {
                Platform* platform = new Platform(realX, realY, 203);
                g_objectList.push_back(platform);

                int cellX = (int)(realX / GRID_CELL_SIZE);
                int cellY = (int)(realY / GRID_CELL_SIZE);

                if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 && cellY < MAX_CELL_ROW) {
                    AddObjectToGrid(platform);
                }
            }
            else if (tileID == 5)
            {
                Breakable* breakableBlock = new Breakable(realX, realY, 205);
                g_objectList.push_back(breakableBlock);

                int cellX = (int)(realX / GRID_CELL_SIZE);
                int cellY = (int)(realY / GRID_CELL_SIZE);

                if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 && cellY < MAX_CELL_ROW) {
                    AddObjectToGrid(breakableBlock);
                }
            }
            else if (tileID == 6)
            {
                LuckyBlock* lucky = new LuckyBlock(realX, realY, 206, 201);
                g_objectList.push_back(lucky);

                int cellX = (int)(realX / GRID_CELL_SIZE);
                int cellY = (int)(realY / GRID_CELL_SIZE);

                if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 && cellY < MAX_CELL_ROW) {
                    AddObjectToGrid(lucky);
                }
            }
        }
    }
    f.close();
}

void AddObjectToGrid(GameObject* obj)
{
    int col = (int)(obj->GetX() / GRID_CELL_SIZE);
    int row = (int)(obj->GetY() / GRID_CELL_SIZE);
    if (col < 0 || col >= MAX_CELL_COL ||
        row < 0 || row >= MAX_CELL_ROW)
        return;
    grid[row][col].push_back(obj);

    obj->gridRow = row;
    obj->gridCol = col;
}

void RemoveObjectFromGrid(GameObject* obj)
{
    if (obj->gridRow < 0 || obj->gridCol < 0)
        return;
    auto& cell = grid[obj->gridRow][obj->gridCol];
    cell.erase(
        std::remove(cell.begin(), cell.end(), obj),
        cell.end()
    );
}

void UpdateObjectGrid(GameObject* obj)
{
    int newCol = (int)(obj->GetX() / GRID_CELL_SIZE);
    int newRow = (int)(obj->GetY() / GRID_CELL_SIZE);

    if (newCol == obj->gridCol &&
        newRow == obj->gridRow)
        return;
    RemoveObjectFromGrid(obj);
    if (newCol < 0 || newCol >= MAX_CELL_COL ||
        newRow < 0 || newRow >= MAX_CELL_ROW)
        return;
    grid[newRow][newCol].push_back(obj);

    obj->gridRow = newRow;
    obj->gridCol = newCol;
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

    textures->Add(TEX_MARIO, L"assets/mario-luigi.png");
    textures->Add(TEX_COMMON1, L"assets/CommonObjects1.png");
    textures->Add(TEX_COMMON2, L"assets/CommonObjects2.png");

    textures->Add(TEX_HUD, L"assets/hud.png");
    textures->Add(TEX_INTRO, L"assets/intro_items.png");
    textures->Add(TEX_BBOX, L"assets/bbox.png");
    textures->Add(TEX_ENEMY_TEST, L"assets/enemy.png");
    textures->Add(TEX_POTION, L"assets/potion.png");


    textures->Add(TEX_LEVEL_CLEAR, L"assets/level-clear.png");
    textures->Add(TEX_YOU_WIN, L"assets/you-win.png");
    textures->Add(TEX_GAME_OVER, L"assets/game-over.png");

    textures->Add(TEX_MAP_LEVEL, L"assets/map-level.png");

    // ==========================================
    // 2. CẮT SPRITES
    // ==========================================

    // Idle
    sprites->Add(0, 115, 45, 126, 59, TEX_MARIO); // Phải
    sprites->Add(1, 70, 45, 81, 59, TEX_MARIO); // Trái

    // Run
    sprites->Add(2, 131, 44, 145, 59, TEX_MARIO);
    sprites->Add(3, 148, 44, 163, 59, TEX_MARIO);

    sprites->Add(4, 51, 44, 65, 59, TEX_MARIO);
    sprites->Add(5, 33, 44, 48, 59, TEX_MARIO);

    // Jump
    sprites->Add(6, 131, 26, 146, 41, TEX_MARIO);
    sprites->Add(7, 50, 26, 65, 41, TEX_MARIO);

    // Skid
    sprites->Add(8, 166, 44, 179, 59, TEX_MARIO);
    sprites->Add(9, 17, 44, 30, 59, TEX_MARIO);

    // ==========================================
    // BIG MARIO SPRITES
    // ==========================================
    // Idle
    sprites->Add(20, 112, 90, 125, 116, TEX_MARIO); // Phải
    sprites->Add(21, 71, 90, 84, 116, TEX_MARIO); // Trái

    // Run
    sprites->Add(22, 129, 90, 144, 116, TEX_MARIO); // Phải 1
    sprites->Add(23, 148, 91, 163, 116, TEX_MARIO); // Phải 2
    sprites->Add(24, 52, 90, 67, 116, TEX_MARIO); // Trái 1
    sprites->Add(25, 33, 91, 48, 116, TEX_MARIO); // Trái 2

    // Jump
    sprites->Add(26, 111, 62, 126, 87, TEX_MARIO); // Phải
    sprites->Add(27, 70, 62, 85, 87, TEX_MARIO); // Trái

    // Skid
    sprites->Add(28, 166, 89, 181, 116, TEX_MARIO); // Phải sang trái (Skid Right)
    sprites->Add(29, 15, 89, 30, 116, TEX_MARIO); // Trái sang phải (Skid Left)

    // Brick
    sprites->Add(10, 435, 152, 450, 167, TEX_COMMON1);

    // Platform
    sprites->Add(11, 481, 152, 496, 167, TEX_COMMON1);

    // Big Block
    sprites->Add(12, 469, 470, 500, 501, TEX_COMMON1);

    // Pipe
    sprites->Add(13, 5, 28, 36, 75, TEX_COMMON2);

    // Breakable
    sprites->Add(14, 453, 152, 468, 167, TEX_COMMON1);

    // Lucky Block
    sprites->Add(15, 185, 7, 200, 22, TEX_COMMON2);

    // Bounding Box
    sprites->Add(99999, 0, 0, 9, 9, TEX_BBOX);

    // Enemy
    sprites->Add(100, 0, 0, 16, 16, TEX_ENEMY_TEST);

    // Potion
    sprites->Add(101, 0, 0, 16, 16, TEX_POTION);

    //Level clear
    sprites->Add(7001, 0, 0, 640, 405, TEX_LEVEL_CLEAR);
    //You win
    sprites->Add(7002, 0, 0, 640, 405, TEX_YOU_WIN);
    //Game over
    sprites->Add(7003, 0, 0, 640, 405, TEX_GAME_OVER);

    //Map level
	sprites->Add(8000, 0, 0, 640, 480, TEX_MAP_LEVEL);

    // ==========================================
    // 3. GOM SPRITES TẠO ANIMATION
    // ==========================================

    ani = new Animation(100); ani->Add(0, 1000); animations->Add(100, ani);
    ani = new Animation(100); ani->Add(1, 1000); animations->Add(101, ani);

    ani = new Animation(100); ani->Add(2); ani->Add(0); ani->Add(3); animations->Add(102, ani);
    ani = new Animation(100); ani->Add(4); ani->Add(1); ani->Add(5); animations->Add(103, ani);

    ani = new Animation(100); ani->Add(6, 1000); animations->Add(104, ani);
    ani = new Animation(100); ani->Add(7, 1000); animations->Add(105, ani);

    ani = new Animation(100); ani->Add(8, 1000); animations->Add(106, ani);
    ani = new Animation(100); ani->Add(9, 1000); animations->Add(107, ani);

    ani = new Animation(100); ani->Add(10, 1000); animations->Add(201, ani);
    ani = new Animation(100); ani->Add(11, 1000); animations->Add(202, ani);
    ani = new Animation(100); ani->Add(12, 1000); animations->Add(203, ani);
    ani = new Animation(100); ani->Add(13, 1000); animations->Add(204, ani);
    ani = new Animation(100); ani->Add(14, 1000); animations->Add(205, ani);
    ani = new Animation(100); ani->Add(15, 1000); animations->Add(206, ani);

    // Big Mario Animations
    ani = new Animation(100); ani->Add(20, 1000); animations->Add(400, ani); // Idle Phải
    ani = new Animation(100); ani->Add(21, 1000); animations->Add(401, ani); // Idle Trái
    ani = new Animation(100); ani->Add(22); ani->Add(20); ani->Add(23); animations->Add(402, ani); // Run Phải
    ani = new Animation(100); ani->Add(24); ani->Add(21); ani->Add(25); animations->Add(403, ani); // Run Trái
    ani = new Animation(100); ani->Add(26, 1000); animations->Add(404, ani); // Jump Phải
    ani = new Animation(100); ani->Add(27, 1000); animations->Add(405, ani); // Jump Trái
    ani = new Animation(100); ani->Add(28, 1000); animations->Add(406, ani); // Skid Left
    ani = new Animation(100); ani->Add(29, 1000); animations->Add(407, ani); // Skid Right

    ani = new Animation(100); ani->Add(100, 1000); animations->Add(300, ani);
    ani = new Animation(100); ani->Add(101, 1000); animations->Add(301, ani);

    // ==========================================
    // 4. KHỞI TẠO OBJECT
    // ==========================================

    Mario* mario = new Mario(100.0f, 200.0f);
    g_objectList.push_back(mario);

    SpawnEnemy(200.0f, 200.0f);

    for (int i = 0; i < 10; i++)
    {
        sprites->Add(1000 + i, 22 + i * 16, 136, 22 + (i + 1) * 16, 136 + 16, TEX_HUD);
    }

    HUD::GetInstance()->LoadSprites();

    // Khởi tạo các Scene và chuyển giao quyền cho SceneManager
    SceneManager::GetInstance()->Init();

    Buff* potion = new Buff(150.0f, 200.0f, 301);
    g_objectList.push_back(potion);
    AddObjectToGrid(potion);

    Flag* flag = new Flag(300.0f, 100.0f);
    g_objectList.push_back(flag);
    AddObjectToGrid(flag);

    Pipe* pipe = new Pipe(15.0f, 80.0f, 204, true, 300.0f, 300.0f);
    g_objectList.push_back(pipe);
    AddObjectToGrid(pipe);

    // ==========================================
    // 5. NẠP VÀ PHÁT ÂM THANH
    // ==========================================
    AudioManager::GetInstance()->LoadSound("win_level", "assets/win-level-complete-mario.mp3");
    AudioManager::GetInstance()->LoadSound("mario_die", "assets/super-mario-death-sound-sound-effect.mp3");
    AudioManager::GetInstance()->LoadSound("mario_jump", "assets/maro-jump-sound-effect.mp3");
    AudioManager::GetInstance()->LoadSound("intro_theme", "assets/Super Mario Bros3 Opening theme.mp3");
    AudioManager::GetInstance()->PlayMusic("intro_theme", true);
}

void Cleanup()
{
    for (GameObject* obj : g_objectList) delete obj;
    g_objectList.clear();

    // Dọn dẹp Scene Manager
    SceneManager::GetInstance()->Cleanup();

    HUD::DestroyInstance();
    Animations::GetInstance()->Clear();
    AudioManager::GetInstance()->CleanUp();

    Game::GetInstance()->ReleaseDirectX();
}

void SpawnEnemy(float x, float y)
{
    Enemy* enemy = new Enemy(x, y, 300);
    g_objectList.push_back(enemy);
    AddObjectToGrid(enemy);
}

#pragma endregion