#pragma region ImportLibaries

#include "animation/Animations.h"
#include "audio/AudioManager.h"
#include "core/Game.h"
#include "gameobject/Breakable.h"
#include "gameobject/Brick.h"
#include "gameobject/Buff.h"
#include "gameobject/Enemy.h"
#include "gameobject/Goomba.h"
#include "gameobject/Koopa.h"

#include "gameobject/GroundBlock.h"
#include "gameobject/LuckyBlock.h"
#include "gameobject/Mario.h"
#include "gameobject/Pipe.h"
#include "gameobject/Platform.h"
#include "gameplay/GameManager.h"
#include "gameplay/SceneManager.h"
#include "render/Camera.h"
#include "render/Sprites.h"
#include "render/Textures.h"
#include "ui/HUD.h"

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

// Kích thước bản đồ (cần khớp với kích thước trong file map)
#define MAP_WIDTH (125 * 15.0f) // 1875.0f
#define MAP_HEIGHT 408.f
#pragma endregion

#pragma region GlobalVariables/GameObjects/ID_Definitions

std::vector<GameObject *> g_objectList;
std::vector<GameObject *> grid[MAX_CELL_ROW][MAX_CELL_COL];

bool g_showBBox = false;

enum TEXTURE_ID {
  TEX_MARIO = 0,
  TEX_COMMON1 = 1,
  TEX_COMMON2 = 2,
  TEX_FIRE_MARIO = 3,
  TEX_SUKUNA_MARIO = 4,
  TEX_ENEMIES_1 = 10, // enemies_transparent.png (Goomba, Green Koopa walk)
  TEX_ENEMIES_2 =
      15, // enemies_transparent_3.png (Koopa shell, Red Koopa, Flying Koopa)
  TEX_HUD = 20,
  TEX_INTRO = 30,
  TEX_BBOX = 99,
  TEX_ENEMY_TEST = 100,
  TEX_POTION = 101,

  TEX_LEVEL_CLEAR = 701,
  TEX_GAME_OVER = 702,
  TEX_YOU_WIN = 703,
  TEX_MAP_LEVEL = 800,
  TEX_OBTAIN_ITEM = 900,
  TEX_WHITE = 999,
	TEX_LAVA_BRICK = 1000,
	TEX_LAVA_BRICK2 = 1001,
	TEX_BLACK_BRICK = 1002,
	TEX_GRASS_BRICK = 1003,
	TEX_CLOUD_BRICK = 1004
};

#pragma endregion

#pragma region FunctionPrototypes

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void LoadMap(LPCWSTR filePath);
void LoadResources();
void Update(DWORD dt);
void Render();
void Cleanup();
void AddObjectToGrid(GameObject *obj);
void RemoveObjectFromGrid(GameObject *obj);
void UpdateObjectGrid(GameObject *obj);
void SpawnEnemy(float x, float y);

#pragma endregion
#pragma region MainFunction

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
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
  LoadResources();

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
                              MAP_WIDTH, MAP_HEIGHT);

  while (msg.message != WM_QUIT) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      ULONGLONG now = GetTickCount64();
      float dt = (float)(now - frameStart);

      if (dt >= tickPerFrame) {
        frameStart = now;

        // --- GAME LOGIC HERE ---
        Update(dt);
        Render();
      } else {
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
void Update(DWORD dt) {
  static bool isPaused = false;
  static bool isF5Pressed = false;
  if (GetAsyncKeyState(VK_F5) & 0x8000) {
    if (!isF5Pressed) {
      isPaused = !isPaused;
      if (isPaused) {
        AudioManager::GetInstance()->PauseMusic();
      } else {
        AudioManager::GetInstance()->ResumeMusic();
      }
      isF5Pressed = true;
    }
  } else {
    isF5Pressed = false;
  }

  if (isPaused) {
    return;
  }

  // F2: Toggle bounding box display
  static bool isF2Pressed = false;
  if (GetAsyncKeyState(VK_F2) & 0x8000) {
    if (!isF2Pressed) {
      g_showBBox = !g_showBBox;
      isF2Pressed = true;
    }
  } else {
    isF2Pressed = false;
  }

  // F3: Toggle debug mode (bỏ qua khoá màn chơi trên World Map)
  static bool isF3Pressed = false;
  if (GetAsyncKeyState(VK_F3) & 0x8000) {
    if (!isF3Pressed) {
      bool current = GameManager::GetInstance()->IsDebugMode();
      GameManager::GetInstance()->SetDebugMode(!current);
      isF3Pressed = true;
    }
  } else {
    isF3Pressed = false;
  }

  // F4: Debug - Chế độ thêm thẻ bài
  static bool isF4Pressed = false;
  static bool cardDebugMode = false;
  if (GetAsyncKeyState(VK_F4) & 0x8000) {
    if (!isF4Pressed) {
      cardDebugMode = !cardDebugMode;
      isF4Pressed = true;
    }
  } else {
    isF4Pressed = false;
  }

  // Xử lý thêm thẻ bài trong chế độ debug
  if (cardDebugMode) {
    GameManager *gm = GameManager::GetInstance();
    static bool isAPressed = false;
    static bool isSPressed = false;
    static bool isDPressed = false;
    static bool isFPressed = false;

    if (GetAsyncKeyState('A') & 0x8000) {
      if (!isAPressed) {
        gm->AddCard(1);
        isAPressed = true;
      }
    } else
      isAPressed = false;

    if (GetAsyncKeyState('S') & 0x8000) {
      if (!isSPressed) {
        gm->AddCard(2);
        isSPressed = true;
      }
    } else
      isSPressed = false;

    if (GetAsyncKeyState('D') & 0x8000) {
      if (!isDPressed) {
        gm->AddCard(3);
        isDPressed = true;
      }
    } else
      isDPressed = false;

    if (GetAsyncKeyState('F') & 0x8000) {
      if (!isFPressed) {
        gm->AddCard(4);
        isFPressed = true;
      }
    } else
      isFPressed = false;
  }

  extern std::vector<GameObject *> g_objectList;
  GameObject *mario = g_objectList.empty() ? NULL : g_objectList[0];
  if (mario && SceneManager::GetInstance()->GetState() == STATE_PLAYING) {
    Camera::GetInstance()->Update(mario->GetX(), mario->GetY(), dt / 1000.0f);
  }

  SceneManager::GetInstance()->Update(dt);
}

// DRAWING (Show on screen)
void Render() {
  Game *game = Game::GetInstance();
  ID3D10Device *dev = game->GetDevice();

  if (dev) {
    float bgColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    if (SceneManager::GetInstance()->GetState() == STATE_PLAYING) {
      int currentLevel = GameManager::GetInstance()->GetLevel();

      if (currentLevel == 2 || currentLevel == 4 || currentLevel == 5) {
        // Đen (Lòng đất, Ban đêm, Lâu đài)
        bgColor[0] = 0.0f;
        bgColor[1] = 0.0f;
        bgColor[2] = 0.0f;
      } else {
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
                         LPARAM lParam) {
  switch (message) {
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

void LoadMap(LPCWSTR filePath) {
  ifstream f;
  f.open(filePath);

  if (!f.is_open())
    return;

  int rows, cols;
  f >> rows >> cols;

  std::vector<std::vector<int>> mapData(rows, std::vector<int>(cols, 0));
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      f >> mapData[r][c];
    }
  }

  int currentLevel = GameManager::GetInstance()->GetLevel();
  int animTop = 211;
  int animBottom = 212;

  if (currentLevel == 2) {
    animTop = 213;
    animBottom = 214;
  } else if (currentLevel == 3) {
    animTop = 217;
    animBottom = 218;
  } else if (currentLevel == 4 || currentLevel == 5) {
    animTop = 215;
    animBottom = 216;
  } else {
    animTop = 211;
    animBottom = 212;
  }

  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      int tileID = mapData[r][c];

      float realX = c * 15.0f;
      float realY = ((rows - r - 1) * 15.0f) + 35.0f;

      if (tileID == 1) {
        if (r >= rows - 2) { // Chỉ áp dụng 2 lớp cho 2 hàng dưới cùng (Mặt đất)
          int groundAnimId = animBottom;
          if (r == 0 || mapData[r - 1][c] != 1) {
            groundAnimId = animTop;
          }

          GroundBlock *ground = new GroundBlock(realX, realY, groundAnimId);
          g_objectList.push_back(ground);

          int cellX = (int)(realX / GRID_CELL_SIZE);
          int cellY = (int)(realY / GRID_CELL_SIZE);

          if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
              cellY < MAX_CELL_ROW) {
            AddObjectToGrid(ground);
          }
        } else {
          // Các khối trên không trung trở thành Gạch (Brick) và hiển thị hình Gạch (201)
          Brick *brick = new Brick(realX, realY, 201);
          g_objectList.push_back(brick);

          int cellX = (int)(realX / GRID_CELL_SIZE);
          int cellY = (int)(realY / GRID_CELL_SIZE);

          if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
              cellY < MAX_CELL_ROW) {
            AddObjectToGrid(brick);
          }
        }
      } else if (tileID == 2) {
        Brick *brick = new Brick(realX, realY, 201);
        g_objectList.push_back(brick);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)(realY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(brick);
        }
      } else if (tileID == 3) {
        Platform *platform = new Platform(realX, realY, 202);
        g_objectList.push_back(platform);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)(realY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(platform);
        }
      } else if (tileID == 4) {
        Platform *platform = new Platform(realX, realY, 203);
        g_objectList.push_back(platform);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)(realY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(platform);
        }
      } else if (tileID == 5) {
        Breakable *breakableBlock = new Breakable(realX, realY, 205);
        g_objectList.push_back(breakableBlock);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)(realY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(breakableBlock);
        }
      } else if (tileID == 6) {
        LuckyBlock *lucky = new LuckyBlock(realX, realY, 206, 201);
        g_objectList.push_back(lucky);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)(realY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(lucky);
        }
      } else if (tileID >= 7 && tileID <= 10) {
        int pipeHeight = tileID - 5; // 7->2, 8->3, 9->4
        Pipe *pipe = new Pipe(realX, realY, pipeHeight, false, 0, 0);
        g_objectList.push_back(pipe);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)(realY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(pipe);
          // Ống nước có thể cao nhiều grid cells
          for (int i = 1; i <= pipeHeight / 4 + 1; i++) {
            if (cellY + i < MAX_CELL_ROW) {
              grid[cellY + i][cellX].push_back(pipe);
            }
          }
        }
      } else if (tileID == 11) {
        Goomba *goomba = new Goomba(realX, realY + 2.0f, GOOMBA_TYPE_NORMAL);
        g_objectList.push_back(goomba);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)((realY + 2.0f) / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(goomba);
        }
      } else if (tileID == 12) {
        Koopa *koopa = new Koopa(realX, realY + 2.0f, KOOPA_TYPE_GREEN);
        g_objectList.push_back(koopa);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)((realY + 2.0f) / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(koopa);
        }
      } else if (tileID == 13) {
        Koopa *koopa = new Koopa(realX, realY + 2.0f, KOOPA_TYPE_RED);
        g_objectList.push_back(koopa);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)((realY + 2.0f) / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(koopa);
        }
      } else if (tileID == 14) {
        Koopa *koopa = new Koopa(realX, realY + 2.0f, KOOPA_TYPE_GREEN_FLYING);
        g_objectList.push_back(koopa);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)((realY + 2.0f) / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(koopa);
        }
      }
    }
  }
  GameManager::GetInstance()->SetMapRightEdge(cols * 15.0f);
  f.close();
}

void AddObjectToGrid(GameObject *obj) {
  int col = (int)(obj->GetX() / GRID_CELL_SIZE);
  int row = (int)(obj->GetY() / GRID_CELL_SIZE);
  if (col < 0 || col >= MAX_CELL_COL || row < 0 || row >= MAX_CELL_ROW)
    return;
  grid[row][col].push_back(obj);

  obj->gridRow = row;
  obj->gridCol = col;
}

void RemoveObjectFromGrid(GameObject *obj) {
  if (obj->gridRow < 0 || obj->gridCol < 0)
    return;
  auto &cell = grid[obj->gridRow][obj->gridCol];
  cell.erase(std::remove(cell.begin(), cell.end(), obj), cell.end());
}

void UpdateObjectGrid(GameObject *obj) {
  int newCol = (int)(obj->GetX() / GRID_CELL_SIZE);
  int newRow = (int)(obj->GetY() / GRID_CELL_SIZE);

  if (newCol == obj->gridCol && newRow == obj->gridRow)
    return;
  RemoveObjectFromGrid(obj);
  if (newCol < 0 || newCol >= MAX_CELL_COL || newRow < 0 ||
      newRow >= MAX_CELL_ROW)
    return;
  grid[newRow][newCol].push_back(obj);

  obj->gridRow = newRow;
  obj->gridCol = newCol;
}

void LoadResources() {
  Textures *textures = Textures::GetInstance();
  Sprites *sprites = Sprites::GetInstance();
  Animations *animations = Animations::GetInstance();
  Animation *ani;

  // ==========================================
  // 1. NẠP TÀI NGUYÊN
  // ==========================================

  textures->Add(TEX_MARIO, L"assets/mario-luigi.png");
  textures->Add(TEX_COMMON1, L"assets/CommonObjects1.png");
  textures->Add(TEX_COMMON2, L"assets/CommonObjects2.png");
  textures->Add(TEX_FIRE_MARIO, L"assets/fireMario.png");
  textures->Add(TEX_SUKUNA_MARIO, L"assets/sukunaMario.png");

  textures->Add(TEX_HUD, L"assets/hud.png");
  textures->Add(TEX_INTRO, L"assets/intro_items.png");
  textures->Add(TEX_BBOX, L"assets/bbox.png");
  textures->Add(TEX_ENEMY_TEST, L"assets/enemy.png");
  textures->Add(TEX_POTION, L"assets/potion.png");

  textures->Add(TEX_LEVEL_CLEAR, L"assets/level-clear.png");
  textures->Add(TEX_YOU_WIN, L"assets/you-win.png");
  textures->Add(TEX_GAME_OVER, L"assets/game-over.png");

  textures->Add(TEX_MAP_LEVEL, L"assets/map-level.png");

  textures->Add(TEX_OBTAIN_ITEM, L"assets/obtain-item.png");
  textures->Add(TEX_WHITE, L"assets/white.png");

  // Enemy sprite sheets
  textures->Add(TEX_ENEMIES_1, L"assets/enemies_transparent.png");
  textures->Add(TEX_ENEMIES_2, L"assets/enemies_transparent_3.png");

  // Brick
  //lava brick
  textures->Add(TEX_LAVA_BRICK, L"assets/lavabrick.png");
  textures->Add(TEX_LAVA_BRICK2, L"assets/lavabrick2.png"); //phần đất phía dưới
  //black brick
  textures->Add(TEX_BLACK_BRICK, L"assets/blackbrick.png");
  //grass brick
  textures->Add(TEX_GRASS_BRICK, L"assets/grassbrick.png");
  //cloud brick
  textures->Add(TEX_CLOUD_BRICK, L"assets/cloudbrick.png");

  // ==========================================
  // 2. CẮT SPRITES
  // ==========================================

  // Idle
  sprites->Add(0, 115, 45, 126, 59, TEX_MARIO); // Phải
  sprites->Add(1, 70, 45, 81, 59, TEX_MARIO);   // Trái

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
  sprites->Add(21, 71, 90, 84, 116, TEX_MARIO);   // Trái

  // Run
  sprites->Add(22, 129, 90, 144, 116, TEX_MARIO); // Phải 1
  sprites->Add(23, 148, 91, 163, 116, TEX_MARIO); // Phải 2
  sprites->Add(24, 52, 90, 67, 116, TEX_MARIO);   // Trái 1
  sprites->Add(25, 33, 91, 48, 116, TEX_MARIO);   // Trái 2

  // Jump
  sprites->Add(26, 111, 62, 126, 87, TEX_MARIO); // Phải
  sprites->Add(27, 70, 62, 85, 87, TEX_MARIO);   // Trái

  // Skid
  sprites->Add(28, 166, 89, 181, 116, TEX_MARIO); // Phải sang trái (Skid Right)
  sprites->Add(29, 15, 89, 30, 116, TEX_MARIO);   // Trái sang phải (Skid Left)

  // Cast
  sprites->Add(30, 71, 119, 84, 136, TEX_MARIO);   // Phải
  sprites->Add(31, 112, 119, 125, 136, TEX_MARIO); // Trái

  // ==========================================
  // FIRE MARIO SPRITES
  // ==========================================
  // Idle
  sprites->Add(40, 216, 2, 229, 28, TEX_FIRE_MARIO); // Phải
  sprites->Add(41, 176, 2, 189, 28, TEX_FIRE_MARIO); // Trái

  // Run
  sprites->Add(42, 255, 2, 270, 28, TEX_FIRE_MARIO); // Phải 1
  sprites->Add(43, 295, 3, 310, 28, TEX_FIRE_MARIO); // Phải 2
  sprites->Add(44, 135, 2, 150, 28, TEX_FIRE_MARIO); // Trái 1
  sprites->Add(45, 95, 3, 110, 28, TEX_FIRE_MARIO);  // Trái 2

  // Jump
  sprites->Add(46, 335, 3, 350, 28, TEX_FIRE_MARIO); // Phải
  sprites->Add(47, 55, 3, 70, 28, TEX_FIRE_MARIO);   // Trái

  // Skid
  sprites->Add(48, 215, 42, 230, 69, TEX_FIRE_MARIO); // Mặt trái (Skid Right)
  sprites->Add(49, 175, 42, 190, 69, TEX_FIRE_MARIO); // Mặt phải (Skid Left)

  // Cast
  sprites->Add(50, 252, 42, 272, 68, TEX_FIRE_MARIO); // Phải
  sprites->Add(51, 133, 42, 153, 68, TEX_FIRE_MARIO); // Trái

  // Fireball
  sprites->Add(601, 178, 124, 185, 131, TEX_FIRE_MARIO);
  sprites->Add(602, 188, 124, 195, 131, TEX_FIRE_MARIO);
  sprites->Add(603, 198, 124, 205, 131, TEX_FIRE_MARIO);
  sprites->Add(604, 208, 124, 215, 131, TEX_FIRE_MARIO);

  // Fireball Explosion
  sprites->Add(605, 182, 138, 189, 145, TEX_FIRE_MARIO);
  sprites->Add(606, 198, 135, 209, 148, TEX_FIRE_MARIO);
  sprites->Add(607, 214, 134, 229, 149, TEX_FIRE_MARIO);

  // FireBlast (Right)
  sprites->Add(608, 8, 74, 79, 97, TEX_FIRE_MARIO);
  sprites->Add(609, 8, 106, 79, 129, TEX_FIRE_MARIO);
  // FireBlast (Left)
  sprites->Add(611, 85, 74, 156, 97, TEX_FIRE_MARIO);
  sprites->Add(612, 85, 106, 156, 129, TEX_FIRE_MARIO);

  // ==========================================
  // SUKUNA MARIO SPRITES
  // ==========================================
  // Idle
  sprites->Add(60, 216, 2, 229, 28, TEX_SUKUNA_MARIO); // Phải
  sprites->Add(61, 176, 2, 189, 28, TEX_SUKUNA_MARIO); // Trái

  // Run
  sprites->Add(62, 255, 2, 270, 28, TEX_SUKUNA_MARIO); // Phải 1
  sprites->Add(63, 295, 3, 310, 28, TEX_SUKUNA_MARIO); // Phải 2
  sprites->Add(64, 135, 2, 150, 28, TEX_SUKUNA_MARIO); // Trái 1
  sprites->Add(65, 95, 3, 110, 28, TEX_SUKUNA_MARIO);  // Trái 2

  // Jump
  sprites->Add(66, 335, 3, 350, 28, TEX_SUKUNA_MARIO); // Phải
  sprites->Add(67, 55, 3, 70, 28, TEX_SUKUNA_MARIO);   // Trái

  // Skid
  sprites->Add(68, 215, 42, 230, 69, TEX_SUKUNA_MARIO); // Mặt trái (Skid Right)
  sprites->Add(69, 175, 42, 190, 69, TEX_SUKUNA_MARIO); // Mặt phải (Skid Left)

  // Cast
  sprites->Add(70, 252, 42, 272, 68, TEX_SUKUNA_MARIO); // Phải
  sprites->Add(71, 133, 42, 153, 68, TEX_SUKUNA_MARIO); // Trái

  // Slash
  sprites->Add(630, 25, 85, 53, 106, TEX_SUKUNA_MARIO);  // short slash
  sprites->Add(631, 72, 75, 117, 106, TEX_SUKUNA_MARIO); // long slash

  // Parry Left
  sprites->Add(632, 382, 113, 396, 140, TEX_SUKUNA_MARIO);
  sprites->Add(633, 356, 113, 371, 140, TEX_SUKUNA_MARIO);
  sprites->Add(634, 331, 114, 347, 140, TEX_SUKUNA_MARIO);

  // Parry Right
  sprites->Add(635, 331, 82, 345, 109, TEX_SUKUNA_MARIO);
  sprites->Add(636, 356, 82, 371, 109, TEX_SUKUNA_MARIO);
  sprites->Add(637, 380, 83, 396, 109, TEX_SUKUNA_MARIO);

      // RollingBall
      sprites->Add(620, 74, 151, 108, 185, TEX_MARIO);

  // Death
  sprites->Add(32, 90, 53, 105, 68, TEX_MARIO);

  // Brick
  sprites->Add(10, 435, 152, 450, 167, TEX_COMMON1);

  //Black Brick
  sprites->Add(110,0, 0,15, 15,TEX_BLACK_BRICK); //phần block phía trên
  sprites->Add(111,0, 16,15, 31,TEX_BLACK_BRICK); //phần block phía dưới

  //Lava Brick
  sprites->Add(112, 0, 0, 15, 15, TEX_LAVA_BRICK); //phần block phía trên
  sprites->Add(113, 0, 0, 15, 15, TEX_LAVA_BRICK2); //phần block phía dưới

  //Grass Brick
  sprites->Add(114, 0, 0, 15, 15, TEX_GRASS_BRICK); //phần block phía trên
  sprites->Add(115, 0, 16, 15, 31, TEX_GRASS_BRICK); //phần block phía dưới

  //Cloud Brick
  sprites->Add(116, 0, 0, 15, 15, TEX_CLOUD_BRICK); 

  // Platform
  sprites->Add(11, 481, 152, 496, 167, TEX_COMMON1);

  // Big Block
  sprites->Add(12, 469, 470, 500, 501, TEX_COMMON1);

  // Pipe (Default 3 blocks) & Supplementary Body
  sprites->Add(13, 5, 28, 36, 73, TEX_COMMON2);
  sprites->Add(16, 5, 60, 36, 75, TEX_COMMON2);

  // Breakable
  sprites->Add(14, 453, 152, 468, 167, TEX_COMMON1);

  // Lucky Block
  sprites->Add(15, 185, 7, 200, 22, TEX_COMMON2);

  // Bounding Box
  sprites->Add(99999, 0, 0, 9, 9, TEX_BBOX);

  // Enemy (Goomba & Koopa sprites)
  sprites->Add(100, 0, 16, 15, 31,
               TEX_ENEMY_TEST); // Giữ nguyên ID 100 làm mặc định nếu cần
  sprites->Add(30001, 0, 16, 15, 31, TEX_ENEMY_TEST);
  sprites->Add(30002, 16, 16, 31, 31, TEX_ENEMY_TEST);
  sprites->Add(30003, 32, 16, 47, 31, TEX_ENEMY_TEST);
  sprites->Add(30101, 96, 0, 111, 31, TEX_ENEMY_TEST);
  sprites->Add(30102, 112, 0, 127, 31, TEX_ENEMY_TEST);
  sprites->Add(30103, 160, 0, 175, 31, TEX_ENEMY_TEST);
  sprites->Add(30104, 176, 0, 191, 31, TEX_ENEMY_TEST);
  sprites->Add(30105, 192, 0, 207, 31, TEX_ENEMY_TEST);
  sprites->Add(30106, 208, 0, 223, 31, TEX_ENEMY_TEST);

  // Potion
  sprites->Add(101, 0, 0, 16, 16, TEX_POTION);

  // Level clear
  sprites->Add(7001, 0, 0, 640, 405, TEX_LEVEL_CLEAR);

  // You win
  sprites->Add(7002, 0, 0, 640, 405, TEX_YOU_WIN);

  // Game over
  sprites->Add(7003, 0, 0, 640, 405, TEX_GAME_OVER);

  // Map level
  sprites->Add(8000, 0, 0, 640, 480, TEX_MAP_LEVEL);

  // Obtain item
  sprites->Add(9000, 0, 0, 640, 480, TEX_OBTAIN_ITEM);

  // White line drawing block sprite
  sprites->Add(99998, 0, 0, 1, 1, TEX_WHITE);

  // ==========================================
  // 3. GOM SPRITES TẠO ANIMATION
  // ==========================================

  // Small Mario Animations
  ani = new Animation(100);
  ani->Add(0, 1000);
  animations->Add(100, ani); // Idle Phải
  ani = new Animation(100);
  ani->Add(1, 1000);
  animations->Add(101, ani); // Idle Trái

  ani = new Animation(100);
  ani->Add(2);
  ani->Add(0);
  ani->Add(3);
  animations->Add(102, ani); // Run Phải
  ani = new Animation(100);
  ani->Add(4);
  ani->Add(1);
  ani->Add(5);
  animations->Add(103, ani); // Run Trái

  ani = new Animation(100);
  ani->Add(6, 1000);
  animations->Add(104, ani); // Jump Phải
  ani = new Animation(100);
  ani->Add(7, 1000);
  animations->Add(105, ani); // Jump Trái

  ani = new Animation(100);
  ani->Add(8, 1000);
  animations->Add(106, ani); // Skid Right
  ani = new Animation(100);
  ani->Add(9, 1000);
  animations->Add(107, ani); // Skid Left

  ani = new Animation(100);
  ani->Add(32, 1000);
  animations->Add(108, ani); // Death

  // Common Objects Animations
  ani = new Animation(100);
  ani->Add(10, 1000);
  animations->Add(201, ani); // Brick
  ani = new Animation(100);
  ani->Add(11, 1000);
  animations->Add(202, ani); // Platform
  ani = new Animation(100);
  ani->Add(12, 1000);
  animations->Add(203, ani); // Big Block
  ani = new Animation(100);
  ani->Add(13, 1000);
  animations->Add(204, ani); // Default Pipe

  ani = new Animation(100);
  ani->Add(16, 1000);
  animations->Add(207, ani); // Supplementary Body
  ani = new Animation(100);
  ani->Add(14, 1000);
  animations->Add(205, ani); // Breakable
  ani = new Animation(100);
  ani->Add(15, 1000);
  animations->Add(206, ani); // Lucky Block

  // Ảnh nền mặt đất (Ground) - 2 lớp
  // Grass (Overworld) - Top
  ani = new Animation(100);
  ani->Add(114, 1000);
  animations->Add(211, ani);
  // Grass (Overworld) - Bottom
  ani = new Animation(100);
  ani->Add(115, 1000);
  animations->Add(212, ani);

  // Black Brick (Underground) - Top
  ani = new Animation(100);
  ani->Add(110, 1000);
  animations->Add(213, ani);
  // Black Brick (Underground) - Bottom
  ani = new Animation(100);
  ani->Add(111, 1000);
  animations->Add(214, ani);

  // Lava Brick (Castle) - Top
  ani = new Animation(100);
  ani->Add(112, 1000);
  animations->Add(215, ani);
  // Lava Brick (Castle) - Bottom
  ani = new Animation(100);
  ani->Add(113, 1000);
  animations->Add(216, ani);

  // Cloud Brick (Athletic/Sky) - Top
  ani = new Animation(100);
  ani->Add(116, 1000);
  animations->Add(217, ani);
  // Cloud Brick (Athletic/Sky) - Bottom
  ani = new Animation(100);
  ani->Add(116, 1000);
  animations->Add(218, ani);

  // Big Mario Animations
  ani = new Animation(100);
  ani->Add(20, 1000);
  animations->Add(400, ani); // Idle Phải
  ani = new Animation(100);
  ani->Add(21, 1000);
  animations->Add(401, ani); // Idle Trái
  ani = new Animation(100);
  ani->Add(22);
  ani->Add(20);
  ani->Add(23);
  animations->Add(402, ani); // Run Phải
  ani = new Animation(100);
  ani->Add(24);
  ani->Add(21);
  ani->Add(25);
  animations->Add(403, ani); // Run Trái
  ani = new Animation(100);
  ani->Add(26, 1000);
  animations->Add(404, ani); // Jump Phải
  ani = new Animation(100);
  ani->Add(27, 1000);
  animations->Add(405, ani); // Jump Trái
  ani = new Animation(100);
  ani->Add(28, 1000);
  animations->Add(406, ani); // Skid Right
  ani = new Animation(100);
  ani->Add(29, 1000);
  animations->Add(407, ani); // Skid Left

  ani = new Animation(100);
  ani->Add(30, 1000);
  animations->Add(408, ani); // Cast Right
  ani = new Animation(100);
  ani->Add(31, 1000);
  animations->Add(409, ani); // Cast Left

  // Fire Mario Animations
  ani = new Animation(100);
  ani->Add(40, 1000);
  animations->Add(500, ani); // Idle Phải
  ani = new Animation(100);
  ani->Add(41, 1000);
  animations->Add(501, ani); // Idle Trái

  ani = new Animation(100);
  ani->Add(42);
  ani->Add(40);
  ani->Add(43);
  animations->Add(502, ani); // Run Phải
  ani = new Animation(100);
  ani->Add(44);
  ani->Add(41);
  ani->Add(45);
  animations->Add(503, ani); // Run Trái

  ani = new Animation(100);
  ani->Add(46, 1000);
  animations->Add(504, ani); // Jump Phải
  ani = new Animation(100);
  ani->Add(47, 1000);
  animations->Add(505, ani); // Jump Trái

  ani = new Animation(100);
  ani->Add(48, 1000);
  animations->Add(506, ani); // Skid Right
  ani = new Animation(100);
  ani->Add(49, 1000);
  animations->Add(507, ani); // Skid Left

  ani = new Animation(100);
  ani->Add(50, 1000);
  animations->Add(508, ani); // Cast Right
  ani = new Animation(100);
  ani->Add(51, 1000);
  animations->Add(509, ani); // Cast Left

  // Sukuna Mario Animations
  ani = new Animation(100);
  ani->Add(60, 1000);
  animations->Add(700, ani); // Idle Phải
  ani = new Animation(100);
  ani->Add(61, 1000);
  animations->Add(701, ani); // Idle Trái

  ani = new Animation(100);
  ani->Add(62);
  ani->Add(60);
  ani->Add(63);
  animations->Add(702, ani); // Run Phải
  ani = new Animation(100);
  ani->Add(64);
  ani->Add(61);
  ani->Add(65);
  animations->Add(703, ani); // Run Trái

  ani = new Animation(100);
  ani->Add(66, 1000);
  animations->Add(704, ani); // Jump Phải
  ani = new Animation(100);
  ani->Add(67, 1000);
  animations->Add(705, ani); // Jump Trái

  ani = new Animation(100);
  ani->Add(68, 1000);
  animations->Add(706, ani); // Skid Right
  ani = new Animation(100);
  ani->Add(69, 1000);
  animations->Add(707, ani); // Skid Left

  ani = new Animation(100);
  ani->Add(70, 1000);
  animations->Add(708, ani); // Cast Right
  ani = new Animation(100);
  ani->Add(71, 1000);
  animations->Add(709, ani); // Cast Left

  // Parry animations
  ani = new Animation(80);
  ani->Add(635);
  ani->Add(636);
  ani->Add(637);
  animations->Add(710, ani); // Parry Right

  ani = new Animation(80);
  ani->Add(632);
  ani->Add(633);
  ani->Add(634);
  animations->Add(711, ani); // Parry Left

  ani = new Animation(50);
  ani->Add(601);
  ani->Add(602);
  ani->Add(603);
  ani->Add(604);
  animations->Add(600, ani); // Fireball

  ani = new Animation(50);
  ani->Add(605);
  ani->Add(606);
  ani->Add(607);
  animations->Add(605, ani); // Fireball Explosion

  ani = new Animation(50);
  ani->Add(608);
  ani->Add(609);
  animations->Add(610, ani); // FireBlast (Right)

  ani = new Animation(50);
  ani->Add(611);
  ani->Add(612);
  animations->Add(611, ani); // FireBlast (Left)

  ani = new Animation(100);
  ani->Add(620);
  animations->Add(620, ani); // RollingBall

  // Items & Enemies Animations
  ani = new Animation(100);
  ani->Add(100, 1000);
  animations->Add(300, ani); // Enemy
  ani = new Animation(100);
  ani->Add(101, 1000);
  animations->Add(301, ani); // Potion

  // ==========================================
  // GOOMBA SPRITES (from enemies_transparent.png = TEX_ENEMIES_1, texID=10)
  // ==========================================
  // Normal Goomba walk
  sprites->Add(31001, 32, 348, 47, 365, TEX_ENEMIES_1);
  sprites->Add(31002, 53, 348, 68, 365, TEX_ENEMIES_1);
  // Normal Goomba flat (die)
  sprites->Add(32001, 11, 356, 26, 365, TEX_ENEMIES_1);
  // Normal Goomba die reverse
  sprites->Add(32002, 170, 348, 185, 365, TEX_ENEMIES_1);

  // ==========================================
  // GREEN KOOPA SPRITES (from enemies_transparent.png = TEX_ENEMIES_1)
  // ==========================================
  // Walk left
  sprites->Add(36001, 71, 66, 86, 93, TEX_ENEMIES_1);
  sprites->Add(36002, 88, 66, 103, 93, TEX_ENEMIES_1);
  // Walk right
  sprites->Add(36501, 638, 66, 653, 93, TEX_ENEMIES_1);
  sprites->Add(36502, 655, 66, 670, 93, TEX_ENEMIES_1);
  // Die (flipped upside-down)
  sprites->Add(39001, 210, 348, 227, 364, TEX_ENEMIES_1);

  // ==========================================
  // GREEN KOOPA SHELL SPRITES (from enemies_transparent_3.png = TEX_ENEMIES_2)
  // ==========================================
  // Shell (sleep)
  sprites->Add(37001, 49, 103, 66, 120, TEX_ENEMIES_2);
  // Shell reverse (sleep upside-down)
  sprites->Add(37501, 328, 103, 347, 120, TEX_ENEMIES_2);
  // Slip (spinning shell)
  sprites->Add(38001, 14, 103, 31, 120, TEX_ENEMIES_2);
  sprites->Add(38002, 14, 128, 31, 145, TEX_ENEMIES_2);
  sprites->Add(38003, 14, 153, 31, 170, TEX_ENEMIES_2);
  // Reborn (shaking shell)
  sprites->Add(40001, 83, 103, 102, 120, TEX_ENEMIES_2);

  // ==========================================
  // RED KOOPA SPRITES (from enemies_transparent_3.png = TEX_ENEMIES_2)
  // ==========================================
  // Walk left
  sprites->Add(106001, 124, 198, 141, 225, TEX_ENEMIES_2);
  sprites->Add(106002, 164, 198, 181, 225, TEX_ENEMIES_2);
  // Walk right
  sprites->Add(106003, 254, 198, 271, 225, TEX_ENEMIES_2);
  sprites->Add(106004, 214, 198, 231, 225, TEX_ENEMIES_2);
  // Red shell (sleep)
  sprites->Add(107001, 49, 203, 66, 220, TEX_ENEMIES_2);
  // Red slip (spinning)
  sprites->Add(108001, 14, 203, 31, 220, TEX_ENEMIES_2);
  sprites->Add(108002, 14, 228, 31, 245, TEX_ENEMIES_2);
  sprites->Add(108003, 14, 253, 31, 270, TEX_ENEMIES_2);
  // Red reborn (shaking)
  sprites->Add(110000, 83, 203, 102, 220, TEX_ENEMIES_2);

  // ==========================================
  // FLYING KOOPA (PARATROOPA) SPRITES (from enemies_transparent_3.png =
  // TEX_ENEMIES_2)
  // ==========================================
  // Jump left
  sprites->Add(71001, 164, 147, 181, 176, TEX_ENEMIES_2);
  sprites->Add(71002, 44, 147, 61, 176, TEX_ENEMIES_2);
  // Jump right
  sprites->Add(71501, 214, 147, 231, 176, TEX_ENEMIES_2);
  sprites->Add(71502, 334, 147, 351, 176, TEX_ENEMIES_2);

  // ==========================================
  // GOOMBA & KOOPA ANIMATIONS
  // ==========================================

  // --- Normal Goomba ---
  ani = new Animation(150);
  ani->Add(31001);
  ani->Add(31002);
  animations->Add(310, ani); // Goomba walk

  ani = new Animation(100);
  ani->Add(32001);
  animations->Add(311, ani); // Goomba flat (die)

  ani = new Animation(100);
  ani->Add(32002);
  animations->Add(316, ani); // Goomba die reverse

  // --- Green Koopa ---
  ani = new Animation(150);
  ani->Add(36001);
  ani->Add(36002);
  animations->Add(312, ani); // Green Koopa walk left

  ani = new Animation(150);
  ani->Add(36501);
  ani->Add(36502);
  animations->Add(330, ani); // Green Koopa walk right

  ani = new Animation(100);
  ani->Add(37001);
  animations->Add(313, ani); // Green Koopa shell static (sleep)

  ani = new Animation(45);
  ani->Add(37001);
  ani->Add(38001);
  ani->Add(38002);
  ani->Add(38003);
  animations->Add(314, ani); // Green Koopa shell spinning (slip)

  ani = new Animation(100);
  ani->Add(37001);
  ani->Add(40001);
  animations->Add(315, ani); // Green Koopa shell shaking (reborn)

  ani = new Animation(100);
  ani->Add(39001);
  animations->Add(331, ani); // Green Koopa die (flipped)

  // --- Red Koopa ---
  ani = new Animation(150);
  ani->Add(106001);
  ani->Add(106002);
  animations->Add(340, ani); // Red Koopa walk left

  ani = new Animation(150);
  ani->Add(106003);
  ani->Add(106004);
  animations->Add(341, ani); // Red Koopa walk right

  ani = new Animation(100);
  ani->Add(107001);
  animations->Add(342, ani); // Red Koopa shell static (sleep)

  ani = new Animation(45);
  ani->Add(107001);
  ani->Add(108001);
  ani->Add(108002);
  ani->Add(108003);
  animations->Add(343, ani); // Red Koopa shell spinning (slip)

  ani = new Animation(100);
  ani->Add(107001);
  ani->Add(110000);
  animations->Add(344, ani); // Red Koopa shell shaking (reborn)

  // --- Flying Koopa (Green Paratroopa) ---
  ani = new Animation(100);
  ani->Add(71001);
  ani->Add(71002);
  animations->Add(350, ani); // Flying Koopa jump left

  ani = new Animation(100);
  ani->Add(71501);
  ani->Add(71502);
  animations->Add(351, ani); // Flying Koopa jump right

  ani = new Animation(100);
  ani->Add(900, 1000);

  // ==========================================
  // 4. KHỞI TẠO
  // ==========================================

  for (int i = 0; i < 10; i++) {
    sprites->Add(1000 + i, 22 + i * 16, 136, 22 + (i + 1) * 16, 136 + 16,
                 TEX_HUD);
  }

  // Khởi tạo các Scene và chuyển giao quyền cho SceneManager
  SceneManager::GetInstance()->Init();

  // ==========================================
  // 5. NẠP VÀ PHÁT ÂM THANH
  // ==========================================
  AudioManager::GetInstance()->LoadSound("mario_theme",
                                         "assets/mario-theme.mp3");
  AudioManager::GetInstance()->LoadSound("level_theme",
                                         "assets/level_theme.mp3");
  AudioManager::GetInstance()->LoadSound("power_up",
                                         "assets/power-up-mario.mp3");
  AudioManager::GetInstance()->LoadSound(
      "mushroom_sound_effect",
      "assets/super_mario_bros_mushroom_sound_effect.mp3");
  AudioManager::GetInstance()->LoadSound("super_mario_pipe",
                                         "assets/super-mario-pipe.mp3");
  AudioManager::GetInstance()->LoadSound("win_level",
                                         "assets/win-level-complete-mario.mp3");
  AudioManager::GetInstance()->LoadSound("use-failed", "assets/use-failed.mp3");
  AudioManager::GetInstance()->LoadSound("fire-blast", "assets/fire-blast.mp3");
  AudioManager::GetInstance()->LoadSound("rolling-ball",
                                         "assets/rolling-ball.mp3");
  AudioManager::GetInstance()->LoadSound(
      "mario_die", "assets/super-mario-death-sound-sound-effect.mp3");
  AudioManager::GetInstance()->LoadSound("mario_jump", "assets/jump.mp3");
  AudioManager::GetInstance()->LoadSound(
      "intro_theme", "assets/Super Mario Bros3 Opening theme.mp3");
  AudioManager::GetInstance()->LoadSound(
      "star_theme", "assets/super-mario-bros-nes-music-star-theme-cut-mp3.mp3");
  AudioManager::GetInstance()->LoadSound("slash-sound",
                                         "assets/slash-sound.mp3");
  AudioManager::GetInstance()->LoadSound("stomp", "assets/stomp.wav");
  AudioManager::GetInstance()->LoadSound("kick", "assets/kick.wav");

  // Phát nhạc intro sau khi tất cả âm thanh đã được nạp xong
  AudioManager::GetInstance()->PlayMusic("intro_theme", true);
}

void Cleanup() {
  for (GameObject *obj : g_objectList)
    delete obj;
  g_objectList.clear();

  // Dọn dẹp Scene Manager
  SceneManager::GetInstance()->Cleanup();

  HUD::DestroyInstance();
  Animations::GetInstance()->Clear();
  AudioManager::GetInstance()->CleanUp();

  Game::GetInstance()->ReleaseDirectX();
}

void SpawnEnemy(float x, float y) {
  Goomba *enemy = new Goomba(x, y, GOOMBA_TYPE_NORMAL);
  g_objectList.push_back(enemy);
  AddObjectToGrid(enemy);
}

#pragma endregion