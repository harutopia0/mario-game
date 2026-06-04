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
    if (newState == STATE_WORLD_MAP) {
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
                HUD::GetInstance()->SetWorld(levelToLoad);
                GameManager::GetInstance()->SetLevel(levelToLoad);

                if (levelToLoad == 2) {
                    LoadMap(L"levels/testmaplevel2.txt");
                }
                else {
                    LoadMap(L"levels/testmap.txt");
                }
                SwitchTo(STATE_PLAYING);
            }
        }
    }
    else if (currentState == STATE_PLAYING) {
        HUD::GetInstance()->Update(dt);
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
        D3DXMatrixScaling(&matZoom, 1.0f, 1.0f, 1.0f); // Render bình thường, không co giãn dồn dập
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