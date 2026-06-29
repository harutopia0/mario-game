#include "SceneManager.h"
#include "Map.h"
#include "../audio/AudioManager.h"
#include "../core/Game.h"
#include "../ui/Intro.h"
#include "../ui/WorldMap.h"
#include "../ui/HUD.h"
#include "../audio/AudioManager.h"
#include "../core/Game.h"
#include "../gameobject/Buff.h"
#include "../gameobject/Enemy.h"

#include "../gameobject/Mario.h"
#include "../gameobject/Pipe.h"
#include "../gameobject/Projectile.h"
#include "../gameobject/RollingBall.h"
#include "../gameplay/GameManager.h"
#include "../render/Camera.h"
#include "../ui/HUD.h"
#include "../ui/Intro.h"
#include "../ui/WorldMap.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdlib.h>
#include <vector>

SceneManager *SceneManager::instance = nullptr;

auto& g_objectList = Map::GetInstance()->GetObjects();

extern bool g_showBBox;




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

  isMarioCastingSkill = false;
  castSkillStartTime = 0;

  isMarioWorldSlashing = false;
  worldSlashStartTime = 0;
  worldSlashOverlayAlpha = 0.0f;
  worldSlashEnemiesKilled = false;

  rouletteCardType = 1;
  lastRouletteTick = 0;
  isRouletteDone = false;
  hitStopTimer = 0.0f;
}

SceneManager *SceneManager::GetInstance() {
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
  Map::GetInstance()->Clear();
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
  } else if (newState == STATE_WORLD_MAP) {
    // Hủy HUD khi về World Map
    HUD::DestroyInstance();

    // Dọn game objects (Mario, enemies, bricks...)
    ClearAllGameObjects();

    if (worldMapScene != nullptr) {
      worldMapScene->Reset();
    }

    AudioManager::GetInstance()->PlayMusic("level_theme", true);
  } else if (newState == STATE_PLAYING) {
    GameManager *gm = GameManager::GetInstance();

    // Reset các giá trị cho màn chơi mới
    gm->ResetForNewLevel();

    // Tạo lại HUD mới
    HUD::GetInstance()->LoadSprites();

    // Tạo Mario mới và khôi phục form từ GameManager
    Mario *mario =
        new Mario(100.0f, 200.0f, gm->IsMarioBig(), gm->IsMarioFire());
    mario->SetSukuna(gm->IsMarioSukuna());

    g_objectList.insert(g_objectList.begin(), mario);

    // Spawn các objects cơ bản cho màn chơi



    // (Ống nước giờ đã được load tự động từ file Level_1.txt qua hàm LoadMap)

    AudioManager::GetInstance()->PlayMusic("mario_theme", true);
  }
}

void SceneManager::ProcessMarioDeath() {
  if (isMarioDying)
    return;

  isMarioDying = true;
  deathStartTime = GetTickCount64();

  AudioManager::GetInstance()->StopMusic();
  AudioManager::GetInstance()->PlaySFX("mario_die");

  GameManager::GetInstance()->SetGameOver(true);
}

void SceneManager::ProcessLevelClear() {
  if (isMarioLevelClearing || isMarioGameWinning)
    return;

  // Lưu form Mario hiện tại vào GameManager trước khi chuyển màn
  Mario *mario =
      g_objectList.empty() ? nullptr : dynamic_cast<Mario *>(g_objectList[0]);
  if (mario != nullptr) {
    GameManager::GetInstance()->SetMarioBig(mario->IsBig());
    GameManager::GetInstance()->SetMarioFire(mario->IsFire());
    GameManager::GetInstance()->SetMarioSukuna(mario->IsSukuna());
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

void SceneManager::ProcessGameWin() {
  if (isMarioGameWinning || isMarioLevelClearing)
    return;

  // Lưu form Mario hiện tại vào GameManager
  Mario *mario =
      g_objectList.empty() ? nullptr : dynamic_cast<Mario *>(g_objectList[0]);
  if (mario != nullptr) {
    GameManager::GetInstance()->SetMarioBig(mario->IsBig());
    GameManager::GetInstance()->SetMarioFire(mario->IsFire());
    GameManager::GetInstance()->SetMarioSukuna(mario->IsSukuna());
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

void SceneManager::ProcessTransform() {
  if (isMarioTransforming)
    return;

  isMarioTransforming = true;
  transformStartTime = GetTickCount64();
}

void SceneManager::Update(DWORD dt) {
  if (hitStopTimer > 0.0f) {
    hitStopTimer -= (float)dt;
    if (hitStopTimer < 0.0f) hitStopTimer = 0.0f;
  }

  if (isMarioDying) {
    if (GetTickCount64() - deathStartTime >= 5000) {
      isMarioDying = false;
      SwitchTo(STATE_INTRO);
      return;
    }
  }

  if (isMarioLevelClearing) {
    DWORD holdingTime = GetTickCount64() - levelClearStartTime;

    if (holdingTime < 4000) {
      if (GetTickCount64() - lastRouletteTick >= 30) {
        rouletteCardType = (rand() % 3) + 1;
        lastRouletteTick = GetTickCount64();
      }
    } else if (!isRouletteDone) {
      rouletteCardType = (rand() % 3) + 1;
      isRouletteDone = true;

      // Chỉ thêm card vào GameManager (HUD đọc trực tiếp từ đó)
      GameManager::GetInstance()->AddCard(rouletteCardType);
    }

    if (holdingTime >= 6000) {
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

  if (isMarioGameWinning) {
    if (GetTickCount64() - gameWinStartTime >= 6000) {
      isMarioGameWinning = false;
      GameManager::GetInstance()->SetGameWin(false);

      SwitchTo(STATE_INTRO);
      return;
    }
  }

  // Tạm dừng game khi Mario đang biến lớn
  if (isMarioTransforming) {
    if (GetTickCount64() - transformStartTime >= MARIO_TRANSFORM_PAUSE_TIME) {
      isMarioTransforming = false;
    } else {
      // Chỉ cập nhật HUD (nhấp nháy PMeter), không cập nhật game objects
      HUD::GetInstance()->Update(dt);
      return;
    }
  }

  // Tạm dừng game 1 giây khi Mario dùng thẻ kỹ năng
  if (isMarioCastingSkill) {
    if (GetTickCount64() - castSkillStartTime >= 1000) {
      isMarioCastingSkill = false;
      AudioManager::GetInstance()->ResumeMusic();
      if (!g_objectList.empty()) {
        Mario *mario = dynamic_cast<Mario *>(g_objectList[0]);
        if (mario != nullptr) {
          mario->SetCastingSkill(false); // Xóa frame vận công
        }
      }
    } else {
      // Chỉ cập nhật HUD, không update game objects
      HUD::GetInstance()->Update(dt);
      return;
    }
  }

  // Xử lý hiệu ứng World Slash
  if (isMarioWorldSlashing) {
    DWORD elapsed = (DWORD)(GetTickCount64() - worldSlashStartTime);

    // Phase 1: Zoom out (0-400ms) - màn hình vẫn sáng bình thường
    if (elapsed < 400) {
      worldSlashOverlayAlpha = 0.0f;
      HUD::GetInstance()->Update(dt);
      return; // freeze game
    }

    // Phase 2: Fade tối (400ms - 800ms) - giữ camera ở góc rộng 1x
    if (elapsed >= 400 && elapsed < 800) {
      worldSlashOverlayAlpha = (elapsed - 400.0f) / 400.0f;
      HUD::GetInstance()->Update(dt);
      return; // freeze game
    }

    // Phase 3: Chém quái trong màn hình góc rộng (800ms - 1200ms) - giữ tối đen
    if (elapsed >= 800 && elapsed < 1200) {
      worldSlashOverlayAlpha = 1.0f;
      if (!worldSlashEnemiesKilled) {
        AudioManager::GetInstance()->PlaySFX("slash-sound");

        // Chỉ tiêu diệt quái vật nằm trong phạm vi camera góc rộng (zoom out 1x)
        float camX = Camera::GetInstance()->GetX();
        float camY = Camera::GetInstance()->GetY();
        float left = camX - 160.0f;
        float right = camX + 480.0f;
        float top = camY - 120.0f;
        float bottom = camY + 360.0f;

        for (size_t i = 0; i < g_objectList.size(); i++) {
          GameObject* obj = g_objectList[i];
          Enemy* enemy = dynamic_cast<Enemy*>(obj);
          if (enemy && !enemy->IsDied()) {
            float el, et, er, eb;
            enemy->GetBoundingBox(el, et, er, eb);
            float ew = er - el;
            float eh = eb - et;

            // Kiểm tra xem enemy có nằm trong vùng nhìn thấy của camera đã zoom out không
            if (el + ew > left && el < right && et + eh > top && et < bottom) {
              enemy->SetDied(true);
            }
          }
        }
        worldSlashEnemiesKilled = true;
      }
      HUD::GetInstance()->Update(dt);
      return; // freeze game
    }

    // Phase 4: Fade sáng lại (1200ms - 1600ms)
    if (elapsed >= 1200 && elapsed < 1600) {
      worldSlashOverlayAlpha = 1.0f - (elapsed - 1200.0f) / 400.0f;
      HUD::GetInstance()->Update(dt);
      return; // freeze game
    }

    // Phase 5: Zoom in lại (1600ms - 2000ms) - màn hình đã sáng
    if (elapsed >= 1600 && elapsed < 2000) {
      worldSlashOverlayAlpha = 0.0f;
      HUD::GetInstance()->Update(dt);
      return; // freeze game
    }

    // Phase 6: Hoàn thành (>= 2000ms)
    if (elapsed >= 2000) {
      isMarioWorldSlashing = false;
      worldSlashOverlayAlpha = 0.0f;
      AudioManager::GetInstance()->ResumeMusic();
      if (!g_objectList.empty()) {
        Mario *mario = dynamic_cast<Mario *>(g_objectList[0]);
        if (mario != nullptr) {
          mario->SetCastingSkill(false);
        }
      }
    }
  }



  if (currentState == STATE_INTRO) {
    if (introScene) {
      introScene->Update(dt);
      if (introScene->IsDone()) {
        SwitchTo(STATE_WORLD_MAP);
      }
    }
  } else if (currentState == STATE_WORLD_MAP) {
    if (worldMapScene) {
      worldMapScene->Update(dt);
      if (worldMapScene->IsDone()) {
        int levelToLoad = worldMapScene->GetSelectedLevel();

        if (levelToLoad > 0) {
          GameManager::GetInstance()->SetLevel(levelToLoad);

          if (levelToLoad == 5) {
            Map::GetInstance()->LoadMap(L"levels/Level_5.txt");
          } else if (levelToLoad == 4) {
            Map::GetInstance()->LoadMap(L"levels/Level_4.txt");
          } else if (levelToLoad == 3) {
            Map::GetInstance()->LoadMap(L"levels/Level_3.txt");
          } else if (levelToLoad == 2) {
            Map::GetInstance()->LoadMap(L"levels/Level_2.txt");
          } else {
            Map::GetInstance()->LoadMap(L"levels/Level_1.txt");
          }
          SwitchTo(STATE_PLAYING);
        }
      }
    }
  } else if (currentState == STATE_PLAYING) {
    // Cập nhật thời gian đếm ngược trong GameManager
    if (!isMarioDying) {
      GameManager::GetInstance()->UpdateTime(dt);
    }

    // Cập nhật HUD (nhấp nháy PMeter)
    HUD::GetInstance()->Update(dt);

    GameObject *marioObj = g_objectList.empty() ? nullptr : g_objectList[0];

    Mario *mario = dynamic_cast<Mario *>(marioObj);

    if (mario) {
      Camera::GetInstance()->Update(mario->GetX(), mario->GetY(), dt / 1000.0f);
    }

    // ==========================================
    // ẤN PHÍM 1/2/3 ĐỂ SỬ DỤNG THẺ BÀI TẠI VỊ TRÍ TƯƠNG ỨNG
    // ==========================================
    for (int slot = 0; slot < 3; slot++) {
      int key = '1' + slot; // '1', '2', '3'
      static bool isSlotPressed[3] = {false, false, false};

      if (GetAsyncKeyState(key) & 0x8000) {
        if (!isSlotPressed[slot]) {
          int cardType = GameManager::GetInstance()->UseCard(slot);
          if (cardType != 0) // CARD_NONE = 0
          {
            Mario *mario = g_objectList.empty()
                               ? nullptr
                               : dynamic_cast<Mario *>(g_objectList[0]);
            if (mario != nullptr && !mario->IsDied()) {
              if (cardType == 3) // CARD_STAR: Bất tử 10 giây
              {
                mario->untouchable = true;
                mario->untouchableStart = GetTickCount64();
                mario->untouchableDuration = 10000;
                mario->isStarInvincible = true;
                AudioManager::GetInstance()->PauseMusic();
                AudioManager::GetInstance()->PlayEventMusic("star_theme", true);
              } else if (cardType ==
                         1) // CARD_MUSHROOM: Biến lớn / Hoặc bắn FireBlast
              {
                if (!mario->IsBig() && !mario->IsFire()) {
                  GameManager::GetInstance()->SetLives(2);
                  mario->SetBig(true);
                } else if (mario->IsFire()) {
                  ProcessMarioCastSkill(cardType, slot);
                } else if (mario->IsBig() && !mario->IsFire()) {
                  ProcessMarioCastSkill(cardType, slot);
                }
              } else if (cardType == 2) // CARD_JOGO: Bắn lửa
              {
                if (!mario->IsFire()) {
                  GameManager::GetInstance()->SetLives(3);
                  mario->SetFire(true);
                } else {
                  ProcessMarioCastSkill(cardType, slot);
                }
              } else if (cardType == 4) // CARD_SUKUNA: Trạng thái Sukuna
              {
                if (!mario->IsSukuna()) {
                  GameManager::GetInstance()->SetLives(3);
                  mario->SetSukuna(true);
                } else {
                  ProcessWorldSlash();
                }
              }
            }
          }
          isSlotPressed[slot] = true;
        }
      } else {
        isSlotPressed[slot] = false;
      }
    }

    // Tối ưu: Lấy vị trí Mario (object đầu tiên) để giới hạn Update cho object
    // static ở xa
    int marioCellX_update = 0;
    int marioCellY_update = 0;
    if (!g_objectList.empty() && g_objectList[0] != nullptr) {
      marioCellX_update = (int)(g_objectList[0]->GetX() / GRID_CELL_SIZE);
      marioCellY_update = (int)(g_objectList[0]->GetY() / GRID_CELL_SIZE);
    }

    size_t numObjects = g_objectList.size();
    for (size_t objIndex = 0; objIndex < numObjects; objIndex++) {
      GameObject *obj = g_objectList[objIndex];
      if (obj->IsDeleted())
        continue;

      // During hit stop, do not update non-effects
      if (hitStopTimer > 0.0f && obj->GetLayer() != LAYER_EFFECTS) {
        continue;
      }

      // Tối ưu: Object static ở xa Mario (ngoài 5 ô lưới = 320px) thì bỏ qua
      // Update. Object động (Mario, đạn Projectile, quái Enemy...) luôn được
      // Update bất kể khoảng cách.
      if (obj->isStatic == true) {
        int objCX = (int)(obj->GetX() / GRID_CELL_SIZE);
        int objCY = (int)(obj->GetY() / GRID_CELL_SIZE);
        if (std::abs(objCX - marioCellX_update) > 5 ||
            std::abs(objCY - marioCellY_update) > 5) {
          continue; // Quá xa Mario, bỏ qua
        }
        obj->Update(dt, NULL);
        continue;
      }
      Map::GetInstance()->UpdateObjectGrid(obj);
      int currentCellX = (int)(obj->GetX() / GRID_CELL_SIZE);
      int currentCellY = (int)(obj->GetY() / GRID_CELL_SIZE);

      std::vector<GameObject *> nearbyObjects;

      for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
          int checkRow = currentCellY + i;
          int checkCol = currentCellX + j;

          if (checkRow >= 0 && checkRow < MAX_CELL_ROW && checkCol >= 0 &&
              checkCol < MAX_CELL_COL) {
            for (GameObject *g : Map::GetInstance()->GetGrid()[checkRow][checkCol]) {
              if (std::find(nearbyObjects.begin(), nearbyObjects.end(), g) ==
                  nearbyObjects.end()) {
                nearbyObjects.push_back(g);
              }
            }
          }
        }
      }
      obj->Update(dt, &nearbyObjects);
    }

    g_objectList.erase(std::remove_if(g_objectList.begin(), g_objectList.end(),
                                      [](GameObject *obj) {
                                        if (obj->IsDeleted()) {
                                          Map::GetInstance()->RemoveObjectFromGrid(obj);
                                          delete obj;
                                          return true;
                                        }
                                        return false;
                                      }),
                       g_objectList.end());
  }
}

void SceneManager::Render() {
  Game *game = Game::GetInstance();
  D3DXMATRIX matZoom;

  if (currentState == STATE_INTRO) {
    Sprite::globalScale = 1.0f;
    D3DXMatrixScaling(&matZoom, 1.0f, 1.0f, 1.0f);
    game->GetSpriteHandler()->SetViewTransform(&matZoom);
    if (introScene)
      introScene->Render();
  } else if (currentState == STATE_WORLD_MAP) {
    Sprite::globalScale = 1.0f;
    D3DXMatrixScaling(&matZoom, 1.0f, 1.0f, 1.0f);
    game->GetSpriteHandler()->SetViewTransform(&matZoom);
    if (worldMapScene)
      worldMapScene->Render();
  } else if (currentState == STATE_PLAYING) {
    Sprite::globalScale = 2.0f;

    D3DXMATRIX matCamera;
    D3DXMATRIX matFinal;

    float zoomScale = 1.0f;
    if (isMarioWorldSlashing) {
      DWORD elapsed = (DWORD)(GetTickCount64() - worldSlashStartTime);
      if (elapsed < 400) {
        zoomScale = 1.0f - (elapsed / 400.0f) * 0.5f; // Smooth zoom out to 0.5f (0-400ms)
      } else if (elapsed >= 400 && elapsed < 1600) {
        zoomScale = 0.5f; // Stay zoomed out (400ms-1600ms)
      } else if (elapsed >= 1600 && elapsed < 2000) {
        zoomScale = 0.5f + ((elapsed - 1600.0f) / 400.0f) * 0.5f; // Smooth zoom in back to 1.0f (1600ms-2000ms)
      }
    }

    D3DXMatrixScaling(&matZoom, 1.0f, 1.0f, 1.0f);
    matCamera = Camera::GetInstance()->GetViewMatrix();

    if (isMarioWorldSlashing) {
      D3DXMATRIX matTranslateToCenter, matScaleRelative, matTranslateBack;
      float relativeScale = zoomScale;

      D3DXMatrixTranslation(&matTranslateToCenter, -320.0f, -240.0f, 0.0f);
      D3DXMatrixScaling(&matScaleRelative, relativeScale, relativeScale, 1.0f);
      D3DXMatrixTranslation(&matTranslateBack, 320.0f, 240.0f, 0.0f);

      matFinal = matCamera * matZoom * matTranslateToCenter * matScaleRelative * matTranslateBack;
    } else {
      float cameraZoom = Camera::GetInstance()->GetZoom();
      if (cameraZoom != 1.0f) {
        D3DXMATRIX matTranslateToCenter, matScaleRelative, matTranslateBack;
        D3DXMatrixTranslation(&matTranslateToCenter, -320.0f, -240.0f, 0.0f);
        D3DXMatrixScaling(&matScaleRelative, cameraZoom, cameraZoom, 1.0f);
        D3DXMatrixTranslation(&matTranslateBack, 320.0f, 240.0f, 0.0f);

        matFinal = matCamera * matZoom * matTranslateToCenter * matScaleRelative * matTranslateBack;
      } else {
        matFinal = matCamera * matZoom;
      }
    }

    game->GetSpriteHandler()->SetViewTransform(&matFinal);

    // RENDER LỚP 2: Hình nền tĩnh cố định trên bản đồ.
    // (Hiện tại chưa có ảnh, sẽ thêm Sprite sau nếu cần)

    // === VÒNG LẶP RENDER THEO LAYER ===
    // Quét từ Layer thấp nhất → cao nhất
    Mario *realMario = nullptr;
    if (!g_objectList.empty()) {
      realMario = dynamic_cast<Mario *>(g_objectList[0]);
    }

    for (int l = LAYER_PROP; l <= LAYER_EFFECTS; l++) {
      for (size_t i = 0; i < g_objectList.size(); i++) {
        GameObject *obj = g_objectList[i];
        if (obj->IsDeleted())
          continue;
        if (obj->GetLayer() != l)
          continue;

        obj->Render();

        // Debug BoundingBox
        if (g_showBBox && realMario != nullptr) {
          bool shouldRenderBBox = false;

          // Ghi chú: Đạn (Projectile) và các class kế thừa (như Fireball,
          // RollingBall...) luôn được hiện hitbox ở mọi khoảng cách
          if (dynamic_cast<Projectile *>(obj)) {
            shouldRenderBBox = true;
          } else {
            int marioCellX = (int)(realMario->GetX() / GRID_CELL_SIZE);
            int marioCellY = (int)(realMario->GetY() / GRID_CELL_SIZE);
            int objCellX = (int)(obj->GetX() / GRID_CELL_SIZE);
            int objCellY = (int)(obj->GetY() / GRID_CELL_SIZE);

            if (std::abs(marioCellX - objCellX) <= 1 &&
                std::abs(marioCellY - objCellY) <= 1) {
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
    D3DXMatrixTranslation(&matUI, 0.0f, -1.0f, 0.0f);
    game->GetSpriteHandler()->SetViewTransform(&matUI);

    Sprite::globalScale = 1.0f;
    HUD::GetInstance()->Render();

    if (isMarioWorldSlashing) {
      RenderWorldSlashOverlay();
    }

    if (isMarioDying) {
      Sprite *gameOverSprite = Sprites::GetInstance()->Get(7003);
      if (gameOverSprite) {
        gameOverSprite->Draw(0.0f, 200.0f);
      }
    } else if (isMarioLevelClearing) {
      // 1. Vẽ màn hình nền "You Got An Item" trước
      Sprite *obtainItemSprite = Sprites::GetInstance()->Get(9000);
      if (obtainItemSprite) {
        obtainItemSprite->Draw(0.0f, 100.0f);
      }

      // 2. Vẽ đè chữ LEVEL CLEAR lên trên ảnh nền
      Sprite *winSprite = Sprites::GetInstance()->Get(7001);
      if (winSprite) {
        winSprite->Draw(0.0f, 200.0f);
      }

      // 3. Vẽ thẻ bài roulette
      int cardSpriteId = 3013 + rouletteCardType;
      Sprite *cardSprite = Sprites::GetInstance()->Get(cardSpriteId);
      if (cardSprite) {
        cardSprite->Draw(480.0f, 320.0f);
      }
    } else if (isMarioGameWinning) {
      Sprite *winGameSprite = Sprites::GetInstance()->Get(7002);
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

void SceneManager::ProcessMarioCastSkill(int cardType, int slot) {
  if (isMarioCastingSkill)
    return;

  if (!g_objectList.empty()) {
    Mario *mario = dynamic_cast<Mario *>(g_objectList[0]);
    if (mario != nullptr) {
      bool skillSuccess = false;

      // Tung chiêu ngay lập tức
      if (cardType == 1 || cardType == 2) { // Mushroom or Jogo Card -> Skill
        if (mario->IsFire()) {
          skillSuccess = mario->ShootFireBlast();
          if (skillSuccess)
            AudioManager::GetInstance()->PlaySFX("fire-blast");
        } else if (cardType == 1 && mario->IsBig() && !mario->IsFire()) {
          skillSuccess = mario->ShootRollingBall();
          if (skillSuccess)
            AudioManager::GetInstance()->PlaySFX("rolling-ball");
        }
      }

      if (skillSuccess) {
        // Đóng băng game 1s
        isMarioCastingSkill = true;
        castSkillStartTime = GetTickCount64();
        mario->SetCastingSkill(true);
        AudioManager::GetInstance()->PauseMusic();
      } else {
        // Hoàn thẻ nếu tung chiêu thất bại (VD: kẹt tường)
        GameManager::GetInstance()->GetHoldingCards()[slot] = cardType;
        AudioManager::GetInstance()->PlaySFX("use-failed");
      }
    }
  }
}

void SceneManager::ProcessWorldSlash() {
  if (isMarioWorldSlashing) return;
  isMarioWorldSlashing = true;
  worldSlashStartTime = GetTickCount64();
  worldSlashOverlayAlpha = 0.0f;
  worldSlashEnemiesKilled = false;
  
  AudioManager::GetInstance()->PauseMusic();

  // Generate random positions, angles, lengths, and thicknesses for the 5 slashes
  for (int i = 0; i < 5; i++) {
    wsX[i] = 100.0f + (float)(rand() % 440); // Center X in [100.0f, 540.0f]
    wsY[i] = 80.0f + (float)(rand() % 320);  // Center Y in [80.0f, 400.0f]
    wsAngle[i] = -1.5f + ((float)rand() / RAND_MAX) * 3.0f; // Angle in [-1.5, 1.5] rad
    wsLength[i] = 450.0f + ((float)rand() / RAND_MAX) * 300.0f; // Length in [450.0f, 750.0f]
    wsThickness[i] = 2.0f + ((float)rand() / RAND_MAX) * 2.5f;  // Thickness in [2.0f, 4.5f]
  }

  if (!g_objectList.empty()) {
    Mario *mario = dynamic_cast<Mario *>(g_objectList[0]);
    if (mario != nullptr) {
      mario->SetCastingSkill(true);
    }
  }
}

void SceneManager::RenderWorldSlashOverlay() {
  if (!isMarioWorldSlashing) return;

  Sprites* sprites = Sprites::GetInstance();
  // Bounding box sprite has ID 99999. Since it's a solid block texture, 
  // modulating with black and alpha makes it a perfect overlay.
  Sprite* blackOverlay = sprites->Get(99999);
  if (blackOverlay) {
    blackOverlay->Draw(0.0f, 0.0f, 640.0f, 480.0f, D3DXCOLOR(0.0f, 0.0f, 0.0f, worldSlashOverlayAlpha));
  }

  // Draw slash lines on top of the black background starting at 800ms
  DWORD elapsed = (DWORD)(GetTickCount64() - worldSlashStartTime);
  if (elapsed >= 800 && elapsed < 1600) {
    float slashAlpha = 1.0f;
    if (elapsed > 1200) {
      slashAlpha = 1.0f - (elapsed - 1200.0f) / 400.0f;
      if (slashAlpha < 0.0f) slashAlpha = 0.0f;
    }

    Sprite* whiteSprite = sprites->Get(99998);
    if (whiteSprite) {
      // Draw 5 clean, sharp white slash lines sequentially (one every 80ms)
      
      // Slash 1 - Appears at 800ms
      if (elapsed >= 800) {
        whiteSprite->DrawRotatedScaled(wsX[0] - 0.5f, wsY[0] - 0.5f, wsAngle[0], wsLength[0], wsThickness[0], D3DXCOLOR(1.0f, 1.0f, 1.0f, slashAlpha));
      }

      // Slash 2 - Appears at 880ms
      if (elapsed >= 880) {
        whiteSprite->DrawRotatedScaled(wsX[1] - 0.5f, wsY[1] - 0.5f, wsAngle[1], wsLength[1], wsThickness[1], D3DXCOLOR(1.0f, 1.0f, 1.0f, slashAlpha));
      }

      // Slash 3 - Appears at 960ms
      if (elapsed >= 960) {
        whiteSprite->DrawRotatedScaled(wsX[2] - 0.5f, wsY[2] - 0.5f, wsAngle[2], wsLength[2], wsThickness[2], D3DXCOLOR(1.0f, 1.0f, 1.0f, slashAlpha));
      }

      // Slash 4 - Appears at 1040ms
      if (elapsed >= 1040) {
        whiteSprite->DrawRotatedScaled(wsX[3] - 0.5f, wsY[3] - 0.5f, wsAngle[3], wsLength[3], wsThickness[3], D3DXCOLOR(1.0f, 1.0f, 1.0f, slashAlpha));
      }

      // Slash 5 - Appears at 1120ms
      if (elapsed >= 1120) {
        whiteSprite->DrawRotatedScaled(wsX[4] - 0.5f, wsY[4] - 0.5f, wsAngle[4], wsLength[4], wsThickness[4], D3DXCOLOR(1.0f, 1.0f, 1.0f, slashAlpha));
      }
    }
  }
}
