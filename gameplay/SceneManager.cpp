#include "SceneManager.h"
#include "../core/Game.h"
#include "../ui/Intro.h"
#include "../ui/WorldMap.h"
#include "../ui/HUD.h"
#include "../audio/AudioManager.h"
#include "../gameplay/GameManager.h"
#include "../gameobject/Mario.h"
#include "../gameobject/Buff.h"
#include "../gameobject/Enemy.h"
#include "../gameobject/Flag.h"
#include "../gameobject/Pipe.h"
#include "../gameobject/Projectile.h"
#include "../gameobject/RollingBall.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdlib.h>

SceneManager* SceneManager::instance = nullptr;

extern std::vector<GameObject*> g_objectList;
extern std::vector<GameObject*> grid[MAX_CELL_ROW][MAX_CELL_COL];
extern bool g_showBBox;
extern void LoadMap(LPCWSTR filePath);
extern void RemoveObjectFromGrid(GameObject* obj);
extern void UpdateObjectGrid(GameObject* obj);
extern void AddObjectToGrid(GameObject* obj);
extern void SpawnEnemy(float x, float y);

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

    rouletteCardType = 1;
    lastRouletteTick = 0;
    isRouletteDone = false;
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

// Hàm dọn sạch toàn bộ game objects và grid
static void ClearAllGameObjects() {
    extern std::vector<GameObject*> g_objectList;
    extern std::vector<GameObject*> grid[MAX_CELL_ROW][MAX_CELL_COL];

    for (GameObject* obj : g_objectList) {
        delete obj;
    }
    g_objectList.clear();

    for (int r = 0; r < MAX_CELL_ROW; r++) {
        for (int c = 0; c < MAX_CELL_COL; c++) {
            grid[r][c].clear();
        }
    }
}

void SceneManager::SwitchTo(GameState newState) {
    currentState = newState;
    if (newState == STATE_INTRO) {
        // Hủy HUD
        HUD::DestroyInstance();

        // Dọn game objects
        ClearAllGameObjects();

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

        // Hủy GameManager để reset hoàn toàn mọi dữ liệu cho game mới
        GameManager::DestroyInstance();

        AudioManager::GetInstance()->PlayMusic("intro_theme", true);
    }
    else if (newState == STATE_WORLD_MAP) {
        // Hủy HUD khi về World Map
        HUD::DestroyInstance();

        // Dọn game objects (Mario, enemies, bricks...)
        ClearAllGameObjects();

        if (worldMapScene != nullptr) {
            worldMapScene->Reset();
        }

        AudioManager::GetInstance()->PlayMusic("level_theme", true);
    }
    else if (newState == STATE_PLAYING) {
        GameManager* gm = GameManager::GetInstance();

        // Reset các giá trị cho màn chơi mới
        gm->ResetForNewLevel();

        // Tạo lại HUD mới
        HUD::GetInstance()->LoadSprites();

        // Tạo Mario mới và khôi phục form từ GameManager
        Mario* mario = new Mario(100.0f, 200.0f, gm->IsMarioBig(), gm->IsMarioFire());

        g_objectList.insert(g_objectList.begin(), mario);

        // Spawn các objects cơ bản cho màn chơi
        SpawnEnemy(200.0f, 200.0f);

        Buff* potion = new Buff(150.0f, 200.0f, 301);
        g_objectList.push_back(potion);
        AddObjectToGrid(potion);

        Flag* flag = new Flag(300.0f, 100.0f);
        g_objectList.push_back(flag);
        AddObjectToGrid(flag);

        Pipe* pipe = new Pipe(15.0f, 80.0f, 204, true, 300.0f, 300.0f);
        g_objectList.push_back(pipe);
        AddObjectToGrid(pipe);

        AudioManager::GetInstance()->PlayMusic("mario_theme", true);
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

    // Lưu form Mario hiện tại vào GameManager trước khi chuyển màn
    Mario* mario = g_objectList.empty() ? nullptr : dynamic_cast<Mario*>(g_objectList[0]);
    if (mario != nullptr) {
        GameManager::GetInstance()->SetMarioBig(mario->IsBig());
        GameManager::GetInstance()->SetMarioFire(mario->IsFire());
    }

    isMarioLevelClearing = true;
    levelClearStartTime = GetTickCount64();

    rouletteCardType = 1;
    lastRouletteTick = GetTickCount64();
    isRouletteDone = false;

    AudioManager::GetInstance()->StopMusic();
    AudioManager::GetInstance()->StopEventMusic();
    AudioManager::GetInstance()->PlaySFX("win_level");

    int clearedLevel = GameManager::GetInstance()->GetLevel();
    GameManager::GetInstance()->SetLevelCleared(clearedLevel, true);

    GameManager::GetInstance()->SetLevelClear(true);
}

void SceneManager::ProcessGameWin()
{
    if (isMarioGameWinning || isMarioLevelClearing)
        return;

    // Lưu form Mario hiện tại vào GameManager
    Mario* mario = g_objectList.empty() ? nullptr : dynamic_cast<Mario*>(g_objectList[0]);
    if (mario != nullptr) {
        GameManager::GetInstance()->SetMarioBig(mario->IsBig());
        GameManager::GetInstance()->SetMarioFire(mario->IsFire());
    }

    isMarioGameWinning = true;
    gameWinStartTime = GetTickCount64();

    AudioManager::GetInstance()->StopMusic();
    AudioManager::GetInstance()->StopEventMusic();
    AudioManager::GetInstance()->PlaySFX("win_level");

    int clearedLevel = GameManager::GetInstance()->GetLevel();
    GameManager::GetInstance()->SetLevelCleared(clearedLevel, true);

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
            SwitchTo(STATE_INTRO);
            return;
        }
    }

    if (isMarioLevelClearing)
    {
        DWORD holdingTime = GetTickCount64() - levelClearStartTime;

        if (holdingTime < 4000)
        {
            if (GetTickCount64() - lastRouletteTick >= 30)
            {
                rouletteCardType = (rand() % 3) + 1;
                lastRouletteTick = GetTickCount64();
            }
        }
        else if (!isRouletteDone)
        {
            rouletteCardType = (rand() % 3) + 1;
            isRouletteDone = true;

            // Chỉ thêm card vào GameManager (HUD đọc trực tiếp từ đó)
            GameManager::GetInstance()->AddCard(rouletteCardType);
        }

        if (holdingTime >= 6000)
        {
            isMarioLevelClearing = false;
            GameManager::GetInstance()->SetLevelClear(false);

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
            // Chỉ cập nhật HUD (nhấp nháy PMeter), không cập nhật game objects
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

                if (levelToLoad > 0) {
                    GameManager::GetInstance()->SetLevel(levelToLoad);

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
        // Cập nhật thời gian đếm ngược trong GameManager
        GameManager::GetInstance()->UpdateTime(dt);

        // Cập nhật HUD (nhấp nháy PMeter)
        HUD::GetInstance()->Update(dt);

        // ==========================================
        // ẤN PHÍM 1/2/3 ĐỂ SỬ DỤNG THẺ BÀI TẠI VỊ TRÍ TƯƠNG ỨNG
        // ==========================================
        for (int slot = 0; slot < 3; slot++)
        {
            int key = '1' + slot; // '1', '2', '3'
            static bool isSlotPressed[3] = { false, false, false };

            if (GetAsyncKeyState(key) & 0x8000)
            {
                if (!isSlotPressed[slot])
                {
                    int cardType = GameManager::GetInstance()->UseCard(slot);
                    if (cardType != 0) // CARD_NONE = 0
                    {
                        Mario* mario = g_objectList.empty() ? nullptr : dynamic_cast<Mario*>(g_objectList[0]);
                        if (mario != nullptr && !mario->IsDied())
                        {
                            if (cardType == 3) // CARD_STAR: Bất tử 10 giây
                            {
                                mario->untouchable = true;
                                mario->untouchableStart = GetTickCount64();
                                mario->untouchableDuration = 10000;
                                mario->isStarInvincible = true;
                                AudioManager::GetInstance()->PauseMusic();
                                AudioManager::GetInstance()->PlayEventMusic("star_theme", true);
                            }
                            else if (cardType == 1) // CARD_MUSHROOM: Biến lớn / Hoặc bắn FireBlast
                            {
                                if (!mario->IsBig() && !mario->IsFire())
                                {
                                    GameManager::GetInstance()->SetLives(2);
                                    mario->SetBig(true);
                                }
                                else if (mario->IsFire())
                                {
                                    mario->ShootFireBlast();
                                }
                                else if (mario->IsBig() && !mario->IsFire())
                                {
                                    mario->ShootRollingBall();
                                }
                            }
                            else if (cardType == 2) // CARD_JOGO: Bắn lửa
                            {
                                if (!mario->IsFire())
                                {
                                    GameManager::GetInstance()->SetLives(3);
                                    mario->SetFire(true);
                                }
                            }
                        }
                    }
                    isSlotPressed[slot] = true;
                }
            }
            else
            {
                isSlotPressed[slot] = false;
            }
        }

        // Tối ưu: Lấy vị trí Mario (object đầu tiên) để giới hạn Update cho object static ở xa
        int marioCellX_update = 0;
        int marioCellY_update = 0;
        if (!g_objectList.empty() && g_objectList[0] != nullptr) {
            marioCellX_update = (int)(g_objectList[0]->GetX() / GRID_CELL_SIZE);
            marioCellY_update = (int)(g_objectList[0]->GetY() / GRID_CELL_SIZE);
        }

        size_t numObjects = g_objectList.size();
        for (size_t objIndex = 0; objIndex < numObjects; objIndex++) {
            GameObject* obj = g_objectList[objIndex];
            if (obj->IsDeleted())
                continue;

            // Tối ưu: Object static ở xa Mario (ngoài 5 ô lưới = 320px) thì bỏ qua Update.
            // Object động (Mario, đạn Projectile, quái Enemy...) luôn được Update bất kể khoảng cách.
            if (obj->isStatic == true) {
                int objCX = (int)(obj->GetX() / GRID_CELL_SIZE);
                int objCY = (int)(obj->GetY() / GRID_CELL_SIZE);
                if (std::abs(objCX - marioCellX_update) > 5 || std::abs(objCY - marioCellY_update) > 5) {
                    continue; // Quá xa Mario, bỏ qua
                }
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
        // === VÒNG LẶP RENDER THEO LAYER ===
        // Quét từ Layer thấp nhất → cao nhất
        Mario* realMario = nullptr;
        if (!g_objectList.empty()) {
            realMario = dynamic_cast<Mario*>(g_objectList[0]);
        }

        for (int l = LAYER_BACKGROUND; l <= LAYER_PLAYER; l++) {
            for (size_t i = 0; i < g_objectList.size(); i++) {
                GameObject* obj = g_objectList[i];
                if (obj->IsDeleted()) continue;
                if (obj->GetLayer() != l) continue;

                obj->Render();

                // Debug BoundingBox
                if (g_showBBox && realMario != nullptr) {
                    bool shouldRenderBBox = false;

                    // Ghi chú: Đạn (Projectile) và các class kế thừa (như Fireball, RollingBall...) luôn được hiện hitbox ở mọi khoảng cách
                    if (dynamic_cast<Projectile*>(obj)) {
                        shouldRenderBBox = true;
                    } else {
                        int marioCellX = (int)(realMario->GetX() / GRID_CELL_SIZE);
                        int marioCellY = (int)(realMario->GetY() / GRID_CELL_SIZE);
                        int objCellX = (int)(obj->GetX() / GRID_CELL_SIZE);
                        int objCellY = (int)(obj->GetY() / GRID_CELL_SIZE);

                        if (std::abs(marioCellX - objCellX) <= 1 && std::abs(marioCellY - objCellY) <= 1) {
                            shouldRenderBBox = true;
                        }
                    }

                    if (shouldRenderBBox) {
                        obj->RenderBoundingBox();
                    }
                }
            }
        }

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
            // 1. Vẽ màn hình nền "You Got An Item" trước
            Sprite* obtainItemSprite = Sprites::GetInstance()->Get(9000);
            if (obtainItemSprite) {
                obtainItemSprite->Draw(0.0f, 100.0f);
            }

            // 2. Vẽ đè chữ LEVEL CLEAR lên trên ảnh nền
            Sprite* winSprite = Sprites::GetInstance()->Get(7001);
            if (winSprite) {
                winSprite->Draw(0.0f, 200.0f);
            }

            // 3. Vẽ thẻ bài roulette
            int cardSpriteId = 3013 + rouletteCardType;
            Sprite* cardSprite = Sprites::GetInstance()->Get(cardSpriteId);
            if (cardSprite) {
                cardSprite->Draw(480.0f, 320.0f);
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