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
#include "ui/Intro.h"
#include "ui/WorldMap.h"
#include "audio/AudioManager.h"
#include "gameplay/GameManager.h"

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
    STATE_WORLD_MAP,
    STATE_PLAYING
};

GameState currentState = STATE_INTRO;
WorldMap* worldMapScene = NULL;

Intro* introScene = NULL;

enum TEXTURE_ID {
    TEX_MARIO = 0,
    TEX_COMMON1 = 1,
    TEX_COMMON2 = 2,
    TEX_HUD = 20,
    TEX_INTRO = 30,
    TEX_BBOX = 99,
    TEX_ENEMY_TEST = 100,
    TEX_POTION = 101,
    TEX_FLAG = 102
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

    if (currentState == STATE_INTRO)
    {
        introScene->Update(dt);
        if (introScene->IsDone())
        {
            currentState = STATE_WORLD_MAP;
        }
    }
    else if (currentState == STATE_WORLD_MAP)
    {
        worldMapScene->Update(dt);
        if (worldMapScene->IsDone())
        {
            currentState = STATE_PLAYING;

            // Lấy level người chơi đã chọn từ World Map
            int levelToLoad = worldMapScene->GetSelectedLevel();

            // CẬP NHẬT HUD VÀ GAMEMANAGER TẠI ĐÂY
            HUD::GetInstance()->SetWorld(levelToLoad);
            GameManager::GetInstance()->SetLevel(levelToLoad);

            // SỬA LẠI TÊN FILE TẠI ĐÂY
            if (levelToLoad == 2)
            {
                LoadMap(L"levels/testmaplevel2.txt");
            }
            else
            {
                LoadMap(L"levels/testmap.txt");
            }
        }
    }
    else {
        HUD::GetInstance()->Update(dt);
        for (GameObject* obj : g_objectList)
        {
            if (obj->IsDeleted())
                continue;
            if (obj->isStatic == true) {
                obj->Update(dt, NULL);
                continue;
            }
            UpdateObjectGrid(obj);
            int currentCellX = (int)(obj->GetX() / GRID_CELL_SIZE);
            int currentCellY = (int)(obj->GetY() / GRID_CELL_SIZE);

            std::vector<GameObject*> nearbyObjects;

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int checkRow = currentCellY + i;
                    int checkCol = currentCellX + j;

                    if (checkRow >= 0 && checkRow < MAX_CELL_ROW && checkCol >= 0 && checkCol < MAX_CELL_COL) {
                        for (GameObject* g : grid[checkRow][checkCol]) {
                            if (std::find(nearbyObjects.begin(),
                                nearbyObjects.end(),
                                g) == nearbyObjects.end())
                            {
                                nearbyObjects.push_back(g);
                            }
                        }
                    }
                }
            }
            obj->Update(dt, &nearbyObjects);
        }

        g_objectList.erase(
            remove_if(
                g_objectList.begin(),
                g_objectList.end(),
                [](GameObject* obj)
                {
                    if (obj->IsDeleted())
                    {
                        RemoveObjectFromGrid(obj);
                        delete obj;
                        return true;
                    }
                    return false;
                }),
            g_objectList.end()
        );
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
        else if (currentState == STATE_WORLD_MAP)
        {
            worldMapScene->Render();
        }
        else
        {
            D3DXMatrixScaling(&matZoom, 2.0f, 2.0f, 1.0f);
            game->GetSpriteHandler()->SetViewTransform(&matZoom);
            GameObject* mario = g_objectList.empty() ? NULL : g_objectList[0];

            // NẾU MARIO ĐANG CHUI ỐNG -> VẼ MARIO ĐẦU TIÊN
            Mario* realMario = dynamic_cast<Mario*>(mario);
            if (realMario != NULL && realMario->isEnteringPipe == true) {
                realMario->Render();
            }

            for (size_t i = 1; i < g_objectList.size(); i++)
            {
                GameObject* obj = g_objectList[i];
                if (obj->IsDeleted())
                    continue;
                obj->Render();

                if (g_showBBox && mario != NULL)
                {
                    int marioCellX = (int)(mario->GetX() / GRID_CELL_SIZE);
                    int marioCellY = (int)(mario->GetY() / GRID_CELL_SIZE);
                    int objCellX = (int)(obj->GetX() / GRID_CELL_SIZE);
                    int objCellY = (int)(obj->GetY() / GRID_CELL_SIZE);

                    if (abs(marioCellX - objCellX) <= 1 && abs(marioCellY - objCellY) <= 1) {
                        obj->RenderBoundingBox();
                    }
                }
            }

            // NẾU MARIO BÌNH THƯỜNG->VẼ MARIO CUỐI CÙNG
            if (realMario != NULL && realMario->isEnteringPipe == false) {
                realMario->Render();
            }

            // Vẽ Bounding Box ở lớp cuối
            if (g_showBBox && mario != NULL) mario->RenderBoundingBox();


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

            float realX = c * 15.0f;

            float realY = ((rows - r - 1) * 15.0f) + 50.0f;


            if (tileID == 1 || tileID == 2)
            {
                Brick* brick = new Brick(realX, realY, 16.0f, 16.0f, 201);
                g_objectList.push_back(brick);

                int cellX = (int)(realX / GRID_CELL_SIZE);
                int cellY = (int)(realY / GRID_CELL_SIZE);

                if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 && cellY < MAX_CELL_ROW) {
                    AddObjectToGrid(brick);
                }
            }
            else if (tileID == 3)
            {
                Platform* platform = new Platform(realX, realY, 16.0f, 16.0f, 202);
                g_objectList.push_back(platform);

                int cellX = (int)(realX / GRID_CELL_SIZE);
                int cellY = (int)(realY / GRID_CELL_SIZE);

                if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 && cellY < MAX_CELL_ROW) {
                    AddObjectToGrid(platform);
                }
            }
            else if (tileID == 4)
            {
                Platform* platform = new Platform(realX, realY, 32.0f, 32.0f, 203);
                g_objectList.push_back(platform);

                int cellX = (int)(realX / GRID_CELL_SIZE);
                int cellY = (int)(realY / GRID_CELL_SIZE);

                if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 && cellY < MAX_CELL_ROW) {
                    AddObjectToGrid(platform);
                }
            }
            else if (tileID == 5)
            {
                Breakable* breakableBlock = new Breakable(realX, realY, 16.0f, 16.0f, 205);
                g_objectList.push_back(breakableBlock);

                int cellX = (int)(realX / GRID_CELL_SIZE);
                int cellY = (int)(realY / GRID_CELL_SIZE);

                if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 && cellY < MAX_CELL_ROW) {
                    AddObjectToGrid(breakableBlock);
                }
            }
            else if (tileID == 6)
            {
                LuckyBlock* lucky = new LuckyBlock(realX, realY, 16.0f, 16.0f, 206, 201);
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

// Thêm object vào grid ngay sau khi được khởi tạo
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

// Xóa object khỏi grid khi nó bị hủy hoặc di chuyển ra khỏi cell cũ
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

// Cập nhật vị trí của object trong grid khi nó di chuyển
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

    // Luckky Block
    sprites->Add(15, 185, 7, 200, 22, TEX_COMMON2);

    // Bounding Box
    sprites->Add(99999, 0, 0, 9, 9, TEX_BBOX);

    // Enemy
    sprites->Add(100, 0, 0, 16, 16, TEX_ENEMY_TEST);
  
    //Potion
    sprites->Add(101, 0, 0, 16, 16, TEX_POTION);

    // ==========================================
    // 3. GOM SPRITES TẠO ANIMATION
    // ==========================================

    // Tạo animation cho Mario
    ani = new Animation(100); ani->Add(0, 1000); animations->Add(100, ani);
    ani = new Animation(100); ani->Add(1, 1000); animations->Add(101, ani);

    ani = new Animation(100); ani->Add(2); ani->Add(0); ani->Add(3); animations->Add(102, ani);
    ani = new Animation(100); ani->Add(4); ani->Add(1); ani->Add(5); animations->Add(103, ani);

    ani = new Animation(100); ani->Add(6, 1000); animations->Add(104, ani);
    ani = new Animation(100); ani->Add(7, 1000); animations->Add(105, ani);
    
    ani = new Animation(100); ani->Add(8, 1000); animations->Add(106, ani);
    ani = new Animation(100); ani->Add(9, 1000); animations->Add(107, ani);

    // Tạo animation cho Brick
    ani = new Animation(100); ani->Add(10, 1000); animations->Add(201, ani);

    // Tạo animation cho Platform
    ani = new Animation(100); ani->Add(11, 1000); animations->Add(202, ani);

    // Tạo animation cho Big Block
    ani = new Animation(100); ani->Add(12, 1000); animations->Add(203, ani);

    // Tạo animation cho Pipe
    ani = new Animation(100); ani->Add(13, 1000); animations->Add(204, ani);

    // Tạo animation cho Breakable
    ani = new Animation(100); ani->Add(14, 1000); animations->Add(205, ani);

    // Tạo animation cho Lucky Block
    ani = new Animation(100); ani->Add(15, 1000); animations->Add(206, ani);

    // Tạo animation cho Enemy
	  ani = new Animation(100); ani->Add(100, 1000); animations->Add(300, ani);

    // ==========================================
    // 4. KHỞI TẠO OBJECT
    // ==========================================

    // Khởi tạo Mario
    Mario* mario = new Mario(100.0f, 200.0f);
    g_objectList.push_back(mario);

    //Khởi tạo Enemy
    SpawnEnemy(200.0f, 200.0f);

    // XÓA DÒNG LOADMAP MẶC ĐỊNH Ở ĐÂY ĐỂ TRÁNH LOAD 2 LẦN
    // LoadMap(L"levels/testmap.txt");

    // Cắt 10 số (0-9)
    for (int i = 0; i < 10; i++)
    {
        sprites->Add(1000 + i, 22 + i * 16, 136, 22 + (i + 1) * 16, 136 + 16, TEX_HUD);
    }

    //HUD
    HUD::GetInstance()->LoadSprites();

    //Intro
    Intro* intro = new Intro();
    intro->LoadSprites();
    //World Map
    worldMapScene = new WorldMap();
    worldMapScene->LoadSprites();
    // Khởi tạo Intro
    introScene = new Intro();
    //Khởi tạo Potion
    Buff* potion = new Buff(150.0f, 200.0f);
    g_objectList.push_back(potion);
    AddObjectToGrid(potion);

    //Khởi tạo cờ
    Flag* flag = new Flag(300.0f, 100.0f);
    g_objectList.push_back(flag);
    AddObjectToGrid(flag);

    Pipe* pipe = new Pipe(15.0f, 80.0f, 32.0f, 48.0f, 204, true, 300.0f, 300.0f);
    g_objectList.push_back(pipe);
    AddObjectToGrid(pipe);

    // ==========================================
    // 5. NẠP VÀ PHÁT ÂM THANH (Thêm toàn bộ đoạn này)
    // ==========================================
    AudioManager::GetInstance()->LoadSound("intro_theme", "assets/Super Mario Bros3 Opening theme.mp3");
    AudioManager::GetInstance()->PlayMusic("intro_theme", true);
}

void Cleanup()
{
    // Dọn dẹp danh sách Object (Mario, Brick,...)
    for (GameObject* obj : g_objectList) delete obj;
    g_objectList.clear();

    // Dọn dẹp Intro
    if (introScene != NULL)
    {
        delete introScene;
        introScene = NULL;
    }

    // Dọn dẹp HUD
    HUD::DestroyInstance();

    // Dọn dẹp các Manager hệ thống
    Animations::GetInstance()->Clear();
    AudioManager::GetInstance()->CleanUp();

    // Dọn dẹp Sprites và Textures (nếu các class này có hàm Clear)
    // Sprites::GetInstance()->Clear();
    // Textures::GetInstance()->Clear();

    Game::GetInstance()->ReleaseDirectX();
}
void SpawnEnemy(float x, float y)
{
    Enemy* enemy = new Enemy(x, y, 16.0f, 16.0f);
    g_objectList.push_back(enemy);
    AddObjectToGrid(enemy);
}

#pragma endregion