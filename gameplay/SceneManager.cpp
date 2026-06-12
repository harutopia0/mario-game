#include "SceneManager.h"
#include "../core/Game.h"
#include "../ui/Intro.h"
#include "../ui/WorldMap.h"
#include "../ui/HUD.h"
#include "../audio/AudioManager.h"
#include "../gameplay/GameManager.h"
#include "../gameobject/Mario.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>

SceneManager* SceneManager::instance = nullptr;

extern std::vector<GameObject*> g_objectList;
extern std::vector<GameObject*> grid[MAX_CELL_ROW][MAX_CELL_COL];
extern bool g_showBBox;
extern void LoadMap(LPCWSTR filePath);
extern void RemoveObjectFromGrid(GameObject* obj);
extern void UpdateObjectGrid(GameObject* obj);

SceneManager::SceneManager() {
    currentState = STATE_INTRO;
    introScene = nullptr;
    worldMapScene = nullptr;
    isMarioDying = false;
    deathStartTime = 0;
    isMarioLevelClearing = false;
    levelClearStartTime = 0;
    isMarioGameWinning = false;
    gameWinStartTime = 0;
    isMarioTransforming = false;
    transformStartTime = 0;
}

SceneManager* SceneManager::GetInstance() {
    if (instance == nullptr) {
        instance = new SceneManager();
    }
    return instance;
}

void SceneManager::Init() {
    introScene = new Intro();
    introScene->LoadSprites();

    worldMapScene = new WorldMap();
    worldMapScene->LoadSprites();

    currentState = STATE_INTRO;
}

void SceneManager::SwitchTo(GameState newState) {
    currentState = newState;
    if (newState == STATE_INTRO) {
        if (introScene != nullptr) {
            delete introScene;
        }
        introScene = new Intro();
        introScene->LoadSprites();

        if (worldMapScene != nullptr) {
            delete worldMapScene;
        }
        worldMapScene = new WorldMap();
        worldMapScene->LoadSprites();

        HUD::GetInstance()->SetWorld(1);
        GameManager::GetInstance()->SetLevel(1);
    }
    else if (newState == STATE_WORLD_MAP) {
        if (worldMapScene != nullptr) {
            worldMapScene->Reset();
        }
    }
}

void SceneManager::ProcessMarioDeath()
{
    if (isMarioDying)
        return;

    isMarioDying = true;
    deathStartTime = GetTickCount64();

    AudioManager::GetInstance()->StopMusic();
    AudioManager::GetInstance()->PlaySFX("mario_die");

    GameManager::GetInstance()->SetGameOver(true);
}

void SceneManager::ProcessLevelClear()
{
    if (isMarioLevelClearing || isMarioGameWinning)
        return;

    isMarioLevelClearing = true;
    levelClearStartTime = GetTickCount64();

    AudioManager::GetInstance()->StopMusic();
    AudioManager::GetInstance()->PlaySFX("win_level");

    // Thêm 1 thẻ bài ngẫu nhiên (Nấm/Hoa/Sao) khi qua màn
    int randomCard = (rand() % 3) + 1;
    HUD::GetInstance()->AddCard(randomCard);

    GameManager::GetInstance()->SetLevelClear(true);
}

void SceneManager::ProcessGameWin()
{
    if (isMarioGameWinning || isMarioLevelClearing)
        return;

    isMarioGameWinning = true;
    gameWinStartTime = GetTickCount64();

    AudioManager::GetInstance()->StopMusic();
    AudioManager::GetInstance()->PlaySFX("win_level");

    GameManager::GetInstance()->SetGameWin(true);
}

void SceneManager::ProcessTransform()
{
    if (isMarioTransforming)
        return;

    isMarioTransforming = true;
    transformStartTime = GetTickCount64();
}

void SceneManager::Update(DWORD dt) {
    if (isMarioDying)
    {
        if (GetTickCount64() - deathStartTime >= 5000)
        {
            isMarioDying = false;
            SwitchTo(STATE_WORLD_MAP);
            return;
        }
    }

    if (isMarioLevelClearing)
    {
        if (GetTickCount64() - levelClearStartTime >= 6000)
        {
            isMarioLevelClearing = false;
            GameManager::GetInstance()->SetLevelClear(false);

            // Tự động chuyển dịch đến node của Level kế tiếp
            if (worldMapScene != nullptr) {
                int nextLevel = worldMapScene->GetSelectedLevel() + 1;
                worldMapScene->SetLevelNode(nextLevel);
            }

            SwitchTo(STATE_WORLD_MAP);
            return;
        }
    }

    if (isMarioGameWinning)
    {
        if (GetTickCount64() - gameWinStartTime >= 6000)
        {
            isMarioGameWinning = false;
            GameManager::GetInstance()->SetGameWin(false);

            SwitchTo(STATE_INTRO);
            return;
        }
    }

    // Tạm dừng game khi Mario đang biến lớn
    if (isMarioTransforming)
    {
        if (GetTickCount64() - transformStartTime >= MARIO_TRANSFORM_PAUSE_TIME)
        {
            isMarioTransforming = false;
        }
        else
        {
            // Chỉ cập nhật HUD (thời gian, nhấp nháy), không cập nhật game objects
            HUD::GetInstance()->Update(dt);
            return;
        }
    }

    if (currentState == STATE_INTRO) {
        if (introScene) {
            introScene->Update(dt);
            if (introScene->IsDone()) {
                SwitchTo(STATE_WORLD_MAP);
            }
        }
    }
    else if (currentState == STATE_WORLD_MAP) {
        if (worldMapScene) {
            worldMapScene->Update(dt);
            if (worldMapScene->IsDone()) {
                int levelToLoad = worldMapScene->GetSelectedLevel();

                // Đảm bảo chỉ xử lý tải màn khi trả về một level hợp lệ (> 0)
                if (levelToLoad > 0) {
                    HUD::GetInstance()->SetWorld(levelToLoad);
                    GameManager::GetInstance()->SetLevel(levelToLoad);

                    // PHÂN LOẠI LOAD FILE MAP ĐẾN LEVEL 5
                    if (levelToLoad == 5) {
                        LoadMap(L"levels/Level_5.txt");
                    }
                    else if (levelToLoad == 4) {
                        LoadMap(L"levels/Level_4.txt");
                    }
                    else if (levelToLoad == 3) {
                        LoadMap(L"levels/Level_3.txt");
                    }
                    else if (levelToLoad == 2) {
                        LoadMap(L"levels/Level_2.txt");
                    }
                    else {
                        LoadMap(L"levels/Level_1.txt");
                    }
                    SwitchTo(STATE_PLAYING);
                }
            }
        }
    }
    else if (currentState == STATE_PLAYING) {
        HUD::GetInstance()->Update(dt);

        // ==========================================
        // ẤN X ĐỂ SỬ DỤNG THẺ BÀI (từ phải sang trái)
        // ==========================================
        static bool isXPressed = false;
        if (GetAsyncKeyState('X') & 0x8000)
        {
            if (!isXPressed)
            {
                int cardType = HUD::GetInstance()->UseCard();
                if (cardType != 0) // CARD_NONE = 0
                {
                    Mario* mario = g_objectList.empty() ? nullptr : dynamic_cast<Mario*>(g_objectList[0]);
                    if (mario != nullptr && !mario->IsDied())
                    {
                        if (cardType == 3) // CARD_STAR: Bất tử 5 giây
                        {
                            mario->untouchable = true;
                            mario->untouchableStart = GetTickCount64();
                            mario->untouchableDuration = 5000;
                        }
                        else if (cardType == 1) // CARD_MUSHROOM: Biến lớn
                        {
                            if (!mario->IsBig())
                            {
                                mario->SetBig(true);
                            }
                        }
                        // cardType == 2 (CARD_FLOWER): Tạm thời chưa có hiệu ứng
                    }
                }
                isXPressed = true;
            }
        }
        else
        {
            isXPressed = false;
        }

        for (GameObject* obj : g_objectList) {
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
                            if (std::find(nearbyObjects.begin(), nearbyObjects.end(), g) == nearbyObjects.end()) {
                                nearbyObjects.push_back(g);
                            }
                        }
                    }
                }
            }
            obj->Update(dt, &nearbyObjects);
        }

        g_objectList.erase(
            std::remove_if(
                g_objectList.begin(),
                g_objectList.end(),
                [](GameObject* obj) {
                    if (obj->IsDeleted()) {
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

void SceneManager::Render() {
    Game* game = Game::GetInstance();
    D3DXMATRIX matZoom;

    if (currentState == STATE_INTRO) {
        D3DXMatrixScaling(&matZoom, 1.0f, 1.0f, 1.0f);
        game->GetSpriteHandler()->SetViewTransform(&matZoom);
        if (introScene) introScene->Render();
    }
    else if (currentState == STATE_WORLD_MAP) {
        D3DXMatrixScaling(&matZoom, 1.0f, 1.0f, 1.0f);
        game->GetSpriteHandler()->SetViewTransform(&matZoom);
        if (worldMapScene) worldMapScene->Render();
    }
    else if (currentState == STATE_PLAYING) {
        D3DXMatrixScaling(&matZoom, 2.0f, 2.0f, 1.0f);
        game->GetSpriteHandler()->SetViewTransform(&matZoom);
        GameObject* mario = g_objectList.empty() ? NULL : g_objectList[0];

        Mario* realMario = dynamic_cast<Mario*>(mario);
        if (realMario != NULL && realMario->isEnteringPipe == true) {
            realMario->Render();
        }

        for (size_t i = 1; i < g_objectList.size(); i++) {
            GameObject* obj = g_objectList[i];
            if (obj->IsDeleted())
                continue;
            obj->Render();

            if (g_showBBox && mario != NULL) {
                int marioCellX = (int)(mario->GetX() / GRID_CELL_SIZE);
                int marioCellY = (int)(mario->GetY() / GRID_CELL_SIZE);
                int objCellX = (int)(obj->GetX() / GRID_CELL_SIZE);
                int objCellY = (int)(obj->GetY() / GRID_CELL_SIZE);

                if (std::abs(marioCellX - objCellX) <= 1 && std::abs(marioCellY - objCellY) <= 1) {
                    obj->RenderBoundingBox();
                }
            }
        }

        if (realMario != NULL && realMario->isEnteringPipe == false) {
            realMario->Render();
        }

        if (g_showBBox && mario != NULL) mario->RenderBoundingBox();

        D3DXMATRIX matUI;
        D3DXMatrixScaling(&matUI, 1.0f, 1.0f, 1.0f);
        game->GetSpriteHandler()->SetViewTransform(&matUI);

        HUD::GetInstance()->Render();

        if (isMarioDying) {
            Sprite* gameOverSprite = Sprites::GetInstance()->Get(7003);
            if (gameOverSprite) {
                gameOverSprite->Draw(0.0f, 200.0f);
            }
        }
        else if (isMarioLevelClearing) {
            Sprite* winSprite = Sprites::GetInstance()->Get(7001);
            if (winSprite) {
                winSprite->Draw(0.0f, 200.0f);
            }
        }
        else if (isMarioGameWinning) {
            Sprite* winGameSprite = Sprites::GetInstance()->Get(7002);
            if (winGameSprite) {
                winGameSprite->Draw(0.0f, 200.0f);
            }
        }
    }
}

void SceneManager::Cleanup() {
    if (introScene != nullptr) {
        delete introScene;
        introScene = nullptr;
    }
    if (worldMapScene != nullptr) {
        delete worldMapScene;
        worldMapScene = nullptr;
    }
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}