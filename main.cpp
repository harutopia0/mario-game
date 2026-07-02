#pragma region ImportLibaries

#include "gameplay/Map.h"

#include "animation/Animations.h"
#include "audio/AudioManager.h"
#include "core/Game.h"
#include "gameobject/Breakable.h"
#include "gameobject/Buff.h"
#include "gameobject/Enemy.h"
#include "gameobject/Goomba.h"
#include "gameobject/Koopa.h"

#include "gameobject/GroundBlock.h"
#include "gameobject/LuckyBlock.h"
#include "gameobject/Mario.h"
#include "gameobject/Pipe.h"
#include "gameobject/Platform.h"

#include "gameobject/Prop.h"
#include "gameobject/PropSpawner.h"
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

#pragma endregion

#pragma region GlobalVariables/GameObjects/ID_Definitions



bool g_showBBox = false;

enum TEXTURE_ID {
  TEX_MARIO = 0,
  TEX_COMMON1 = 1,
  TEX_COMMON2 = 2,
  TEX_FIRE_MARIO = 3,
  TEX_SCISSORS_MARIO = 4,
  TEX_ENEMIES_1 = 10, // enemies_transparent.png (Goomba, Green Koopa walk)
  TEX_ENEMIES_2 =
      15, // enemies_transparent_3.png (Koopa shell, Red Koopa, Flying Koopa)
  TEX_HUD = 20,
  TEX_INTRO = 30,
  TEX_BBOX = 99,
  TEX_ENEMY_TEST = 100,

  TEX_LEVEL_CLEAR = 701,
  TEX_GAME_OVER = 702,
  TEX_YOU_WIN = 703,
  TEX_MAP_LEVEL = 800,
  TEX_OBTAIN_ITEM = 900,
  TEX_WHITE = 999,

  TEX_GRASS_BRICK = 1003,
  TEX_CLOUDS = 1005
};

#pragma endregion

#pragma region FunctionPrototypes

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void LoadResources();
void Update(DWORD dt);
void Render();
void Cleanup();

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
                              1000.0f, 408.0f);

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

  // Dọn dẹp
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

  // Removed redundant camera update here (now handled safely in SceneManager::Update)

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
  case WM_KEYDOWN:
    {
      bool isRepeat = (lParam & (1 << 30)) != 0;
      if (!isRepeat) {
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
  textures->Add(TEX_SCISSORS_MARIO, L"assets/sukunaMario.png");

  textures->Add(TEX_HUD, L"assets/hud.png");
  textures->Add(TEX_INTRO, L"assets/intro_items.png");
  textures->Add(TEX_BBOX, L"assets/bbox.png");
  textures->Add(TEX_ENEMY_TEST, L"assets/enemy.png");

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

  // grass brick
  textures->Add(TEX_GRASS_BRICK, L"assets/tiles.png");
  textures->Add(TEX_CLOUDS, L"assets/clouds.png");

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
  // SCISSORS MARIO SPRITES
  // ==========================================
  // Idle
  sprites->Add(60, 216, 2, 229, 28, TEX_SCISSORS_MARIO); // Phải
  sprites->Add(61, 176, 2, 189, 28, TEX_SCISSORS_MARIO); // Trái

  // Run
  sprites->Add(62, 255, 2, 270, 28, TEX_SCISSORS_MARIO); // Phải 1
  sprites->Add(63, 295, 3, 310, 28, TEX_SCISSORS_MARIO); // Phải 2
  sprites->Add(64, 135, 2, 150, 28, TEX_SCISSORS_MARIO); // Trái 1
  sprites->Add(65, 95, 3, 110, 28, TEX_SCISSORS_MARIO);  // Trái 2

  // Jump
  sprites->Add(66, 335, 3, 350, 28, TEX_SCISSORS_MARIO); // Phải
  sprites->Add(67, 55, 3, 70, 28, TEX_SCISSORS_MARIO);   // Trái

  // Skid
  sprites->Add(68, 215, 42, 230, 69, TEX_SCISSORS_MARIO); // Mặt trái (Skid Right)
  sprites->Add(69, 175, 42, 190, 69, TEX_SCISSORS_MARIO); // Mặt phải (Skid Left)

  // Cast
  sprites->Add(70, 252, 42, 272, 68, TEX_SCISSORS_MARIO); // Phải
  sprites->Add(71, 133, 42, 153, 68, TEX_SCISSORS_MARIO); // Trái

  // Slash
  sprites->Add(630, 25, 85, 53, 106, TEX_SCISSORS_MARIO);  // short slash
  sprites->Add(631, 72, 75, 117, 106, TEX_SCISSORS_MARIO); // long slash

  // Parry Left
  sprites->Add(632, 382, 113, 396, 140, TEX_SCISSORS_MARIO);
  sprites->Add(633, 356, 113, 371, 140, TEX_SCISSORS_MARIO);
  sprites->Add(634, 331, 114, 347, 140, TEX_SCISSORS_MARIO);

  // Parry Right
  sprites->Add(635, 331, 82, 345, 109, TEX_SCISSORS_MARIO);
  sprites->Add(636, 356, 82, 371, 109, TEX_SCISSORS_MARIO);
  sprites->Add(637, 380, 83, 396, 109, TEX_SCISSORS_MARIO);

  // RollingBall
  sprites->Add(620, 74, 151, 108, 185, TEX_MARIO);

  // Death
  sprites->Add(32, 90, 53, 105, 68, TEX_MARIO);

  // Brick
  sprites->Add(10, 74, 34, 89, 49, TEX_COMMON1);



  // Grass Brick
  sprites->Add(114, 18, 1, 33, 16, TEX_GRASS_BRICK);   // phần block phía trên
  sprites->Add(115, 18, 18, 33, 33, TEX_GRASS_BRICK);  // phần block phía dưới
  sprites->Add(1141, 1, 1, 16, 16, TEX_GRASS_BRICK);   // Top Left
  sprites->Add(1142, 35, 1, 50, 16, TEX_GRASS_BRICK);  // Top Right
  sprites->Add(1151, 1, 18, 16, 33, TEX_GRASS_BRICK);  // Bottom Left
  sprites->Add(1152, 35, 18, 50, 33, TEX_GRASS_BRICK); // Bottom Right
  sprites->Add(1143, 52, 1, 67, 16, TEX_GRASS_BRICK);  // Top Both
  sprites->Add(1153, 52, 18, 67, 33, TEX_GRASS_BRICK); // Bottom Both

  // Staircase (Grass alternative)
  sprites->Add(1240, 18, 103, 33, 118, TEX_GRASS_BRICK); // Top Center
  sprites->Add(1241, 1, 103, 16, 118, TEX_GRASS_BRICK);  // Top Left
  sprites->Add(1242, 35, 103, 50, 118, TEX_GRASS_BRICK); // Top Right
  sprites->Add(1243, 52, 103, 67, 118, TEX_GRASS_BRICK); // Top Isolated

  // Floating Grass Platform
  sprites->Add(1244, 1, 52, 16, 67, TEX_GRASS_BRICK);  // Floating Left
  sprites->Add(1245, 18, 52, 33, 67, TEX_GRASS_BRICK); // Floating Center
  sprites->Add(1246, 35, 52, 50, 67, TEX_GRASS_BRICK); // Floating Right
  sprites->Add(1247, 52, 52, 67, 67, TEX_GRASS_BRICK); // Floating Isolated

  sprites->Add(1250, 18, 120, 33, 135, TEX_GRASS_BRICK); // Middle Center
  sprites->Add(1251, 1, 120, 16, 135, TEX_GRASS_BRICK);  // Middle Left
  sprites->Add(1252, 35, 120, 50, 135, TEX_GRASS_BRICK); // Middle Right
  sprites->Add(1253, 52, 120, 67, 135, TEX_GRASS_BRICK); // Middle Isolated

  // Cloud Brick fallback to Grass Brick
  sprites->Add(116, 0, 0, 15, 15, TEX_GRASS_BRICK);

  // Level 5 Brick
  sprites->Add(118, 86, 1, 101, 16, TEX_GRASS_BRICK);

  // Cloud Platform Sprites
  sprites->Add(1300, 2, 30, 61, 44, TEX_CLOUDS); // 60px cloud
  sprites->Add(1301, 2, 46, 76, 60, TEX_CLOUDS); // 75px cloud
  sprites->Add(1302, 2, 64, 91, 78, TEX_CLOUDS); // 90px cloud
  sprites->Add(1303, 87, 2, 102, 17, TEX_CLOUDS); // 16x16 cloud block

  // Platform
  sprites->Add(11, 481, 152, 496, 167, TEX_COMMON1);

  // Big Block
  sprites->Add(12, 469, 470, 500, 501, TEX_COMMON1);

  // Pipe (Default 3 blocks) & Supplementary Body
  sprites->Add(13, 5, 28, 36, 73, TEX_COMMON2);
  sprites->Add(16, 5, 60, 36, 75, TEX_COMMON2);

  // Breakable
  sprites->Add(14, 91, 34, 106, 49, TEX_COMMON1);
  sprites->Add(141, 91, 34, 106, 49, TEX_COMMON1);
  sprites->Add(142, 91, 34, 106, 49, TEX_COMMON1);
  sprites->Add(143, 91, 34, 106, 49, TEX_COMMON1);

  // Lucky Block
  sprites->Add(15, 74, 17, 89, 32, TEX_COMMON1);
  sprites->Add(151, 91, 17, 106, 32, TEX_COMMON1);
  sprites->Add(152, 108, 17, 123, 32, TEX_COMMON1);
  sprites->Add(153, 125, 17, 140, 32, TEX_COMMON1);

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



  // Jungle Props
  sprites->Add(81001, 55, 7, 70, 22, TEX_COMMON2); // Bush
  sprites->Add(81002, 3, 80, 130, 143, TEX_COMMON2); // Cliff
  sprites->Add(81003, 59, 27, 121, 74, TEX_COMMON2); // Small cliff
  
  // Underground/Athletic Props (Level 2 & 4)
  sprites->Add(82001, 370, 745, 380, 754, TEX_COMMON2); // Nấm
  sprites->Add(82002, 427, 743, 474, 806, TEX_COMMON2); // Vách đá 1
  sprites->Add(82003, 476, 743, 523, 806, TEX_COMMON2); // Vách đá 2
  sprites->Add(82004, 525, 743, 572, 806, TEX_COMMON2); // Cột đầu lâu
  sprites->Add(82005, 583, 742, 590, 752, TEX_COMMON2); // Mây nhỏ màn 2 & 4
  sprites->Add(85001, 583, 298, 598, 329, TEX_COMMON2); // Cửa sổ màn 5
  sprites->Add(85002, 219, 770, 232, 797, TEX_COMMON2); // Nến màn 5
  
  // Clouds
  sprites->Add(81004, 52, 3, 83, 26, TEX_CLOUDS); // Single cloud
  sprites->Add(81005, 1, 3, 48, 26, TEX_CLOUDS);  // Double cloud

  // Piranha Plant
  sprites->Add(30001, 15, 349, 30, 373, TEX_ENEMIES_2);
  sprites->Add(30002, 65, 350, 80, 373, TEX_ENEMIES_2);

  // Venus Fire Trap (Bottom-Up)
  sprites->Add(30011, 164, 399, 180, 423, TEX_ENEMIES_2); // Trái lên ngậm
  sprites->Add(30012, 64, 399, 80, 423, TEX_ENEMIES_2);   // Trái lên mở
  sprites->Add(30013, 215, 399, 230, 423, TEX_ENEMIES_2); // Phải lên ngậm
  sprites->Add(30014, 315, 399, 330, 423, TEX_ENEMIES_2); // Phải lên mở
  sprites->Add(30015, 115, 399, 130, 423, TEX_ENEMIES_2); // Trái xuống ngậm
  sprites->Add(30016, 15, 399, 30, 423, TEX_ENEMIES_2);   // Trái xuống mở
  sprites->Add(30017, 264, 399, 279, 423, TEX_ENEMIES_2); // Phải xuống ngậm
  sprites->Add(30018, 364, 399, 379, 423, TEX_ENEMIES_2); // Phải xuống mở

  // Venus Fire Trap (Top-Down)
  sprites->Add(30021, 115, 449, 130, 473, TEX_ENEMIES_2); // Trái lên ngậm
  sprites->Add(30022, 15, 449, 30, 473, TEX_ENEMIES_2);   // Trái lên mở
  sprites->Add(30023, 264, 449, 279, 473, TEX_ENEMIES_2); // Phải lên ngậm
  sprites->Add(30024, 364, 449, 379, 473, TEX_ENEMIES_2); // Phải lên mở
  sprites->Add(30025, 164, 449, 180, 473, TEX_ENEMIES_2); // Trái xuống ngậm
  sprites->Add(30026, 64, 449, 80, 473, TEX_ENEMIES_2);   // Trái xuống mở
  sprites->Add(30027, 214, 449, 230, 473, TEX_ENEMIES_2); // Phải xuống ngậm
  sprites->Add(30028, 314, 449, 330, 473, TEX_ENEMIES_2); // Phải xuống mở

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

  // Piranha Plant
  ani = new Animation(150);
  ani->Add(30001);
  ani->Add(30002);
  animations->Add(5000, ani);

  // Venus Fire Trap (Bottom-Up)
  ani = new Animation(150); ani->Add(30011); ani->Add(30012); animations->Add(5001, ani);
  ani = new Animation(150); ani->Add(30013); ani->Add(30014); animations->Add(5002, ani);
  ani = new Animation(150); ani->Add(30015); ani->Add(30016); animations->Add(5003, ani);
  ani = new Animation(150); ani->Add(30017); ani->Add(30018); animations->Add(5004, ani);

  // Venus Fire Trap (Top-Down)
  ani = new Animation(150); ani->Add(30021); ani->Add(30022); animations->Add(5011, ani);
  ani = new Animation(150); ani->Add(30023); ani->Add(30024); animations->Add(5012, ani);
  ani = new Animation(150); ani->Add(30025); ani->Add(30026); animations->Add(5013, ani);
  ani = new Animation(150); ani->Add(30027); ani->Add(30028); animations->Add(5014, ani);

  // Common Objects Animations
  ani = new Animation(100);
  ani->Add(10, 1000);
  animations->Add(201, ani); // Brick
  ani = new Animation(100);
  ani->Add(11, 1000);
  animations->Add(202, ani); // Platform
  // Cloud Platforms
  ani = new Animation(100); ani->Add(1300); animations->Add(1300, ani); // Cloud 60px
  ani = new Animation(100); ani->Add(1301); animations->Add(1301, ani); // Cloud 75px
  ani = new Animation(100); ani->Add(1302); animations->Add(1302, ani); // Cloud 90px
  ani = new Animation(100); ani->Add(1303); animations->Add(1303, ani); // Cloud Block

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
  ani->Add(14, 200);
  ani->Add(141, 200);
  ani->Add(142, 200);
  ani->Add(143, 200);
  animations->Add(205, ani); // Breakable
  ani = new Animation(100);
  ani->Add(15, 200);
  ani->Add(151, 200);
  ani->Add(152, 200);
  ani->Add(153, 200);
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

  // Grass Edge Animations
  ani = new Animation(100);
  ani->Add(1141, 1000);
  animations->Add(219, ani);
  ani = new Animation(100);
  ani->Add(1142, 1000);
  animations->Add(220, ani);
  ani = new Animation(100);
  ani->Add(1151, 1000);
  animations->Add(221, ani);
  ani = new Animation(100);
  ani->Add(1152, 1000);
  animations->Add(222, ani);
  ani = new Animation(100);
  ani->Add(1143, 1000);
  animations->Add(223, ani);
  ani = new Animation(100);
  ani->Add(1153, 1000);
  animations->Add(224, ani);

  // Staircase Edge Animations
  ani = new Animation(100);
  ani->Add(1240, 1000);
  animations->Add(225, ani); // Top Center
  ani = new Animation(100);
  ani->Add(1241, 1000);
  animations->Add(226, ani); // Top Left
  ani = new Animation(100);
  ani->Add(1242, 1000);
  animations->Add(227, ani); // Top Right
  ani = new Animation(100);
  ani->Add(1243, 1000);
  animations->Add(228, ani); // Top Isolated

  ani = new Animation(100);
  ani->Add(1244, 1000);
  animations->Add(236, ani); // Floating Left
  ani = new Animation(100);
  ani->Add(1245, 1000);
  animations->Add(235, ani); // Floating Center
  ani = new Animation(100);
  ani->Add(1246, 1000);
  animations->Add(237, ani); // Floating Right
  ani = new Animation(100);
  ani->Add(1247, 1000);
  animations->Add(238, ani); // Floating Isolated

  ani = new Animation(100);
  ani->Add(1250, 1000);
  animations->Add(229, ani); // Mid Center
  ani = new Animation(100);
  ani->Add(1251, 1000);
  animations->Add(230, ani); // Mid Left
  ani = new Animation(100);
  ani->Add(1252, 1000);
  animations->Add(231, ani); // Mid Right
  ani = new Animation(100);
  ani->Add(1253, 1000);
  animations->Add(232, ani); // Mid Isolated





  // Cloud Brick (Athletic/Sky) - Top
  ani = new Animation(100);
  ani->Add(116, 1000);
  animations->Add(217, ani);
  // Cloud Brick (Athletic/Sky) - Bottom
  ani = new Animation(100);
  ani->Add(116, 1000);
  animations->Add(218, ani);

  // Level 5 Brick
  ani = new Animation(100);
  ani->Add(118, 1000);
  animations->Add(250, ani);
  ani = new Animation(100);
  ani->Add(118, 1000);
  animations->Add(251, ani);

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

  // Scissors Mario Animations
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

  // Items Sprites
  sprites->Add(3018, 1019, 74, 1074, 139, TEX_HUD); // 3018: Nấm
  sprites->Add(3019, 1075, 74, 1130, 139, TEX_HUD); // 3019: Flower
  sprites->Add(3020, 1131, 74, 1186, 139, TEX_HUD); // 3020: Sao
  sprites->Add(3021, 1019, 140, 1074, 205, TEX_HUD); // 3021: Scissors

  // Items & Enemies Animations
  ani = new Animation(100);
  ani->Add(100, 1000);
  animations->Add(300, ani); // Enemy

  ani = new Animation(100);
  ani->Add(3018, 1000);
  animations->Add(301, ani); // Mushroom

  ani = new Animation(100);
  ani->Add(3019, 1000);
  animations->Add(302, ani); // Flower

  ani = new Animation(100);
  ani->Add(3020, 1000);
  animations->Add(303, ani); // Star

  ani = new Animation(100);
  ani->Add(3021, 1000);
  animations->Add(304, ani); // Scissors

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
  // HAMMER BRO & HAMMER SPRITES
  // ==========================================
  // Hammer Bro Walk Right
  sprites->Add(50001, 165, 850, 180, 873, TEX_ENEMIES_2); 
  sprites->Add(50002, 215, 850, 230, 873, TEX_ENEMIES_2); 
  // Hammer Bro Throw Right
  sprites->Add(50003, 265, 850, 280, 873, TEX_ENEMIES_2); 
  
  // Hammer Bro Walk Left
  sprites->Add(50011, 115, 850, 130, 873, TEX_ENEMIES_2); 
  sprites->Add(50012, 65, 850, 80, 873, TEX_ENEMIES_2);   
  // Hammer Bro Throw Left
  sprites->Add(50013, 15, 850, 30, 873, TEX_ENEMIES_2);   

  // Hammer
  sprites->Add(51001, 305, 842, 320, 855, TEX_ENEMIES_2);
  sprites->Add(51002, 329, 840, 342, 855, TEX_ENEMIES_2);
  sprites->Add(51003, 353, 840, 366, 855, TEX_ENEMIES_2);
  sprites->Add(51004, 375, 842, 390, 855, TEX_ENEMIES_2);
  sprites->Add(51005, 305, 869, 320, 882, TEX_ENEMIES_2);
  sprites->Add(51006, 329, 867, 342, 882, TEX_ENEMIES_2);
  sprites->Add(51007, 353, 867, 366, 882, TEX_ENEMIES_2);
  sprites->Add(51008, 375, 869, 390, 882, TEX_ENEMIES_2);


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

  // --- Hammer Bro & Hammer ---
  ani = new Animation(50);
  ani->Add(51001);
  ani->Add(51002);
  ani->Add(51003);
  ani->Add(51004);
  ani->Add(51008);
  ani->Add(51007);
  ani->Add(51006);
  ani->Add(51005);
  animations->Add(11000, ani); // Hammer spinning

  ani = new Animation(150);
  ani->Add(50011);
  ani->Add(50012);
  animations->Add(11001, ani); // Hammer Bro walk left

  ani = new Animation(150);
  ani->Add(50001);
  ani->Add(50002);
  animations->Add(11002, ani); // Hammer Bro walk right

  ani = new Animation(150);
  ani->Add(50013);
  animations->Add(11003, ani); // Hammer Bro throw left

  ani = new Animation(150);
  ani->Add(50003);
  animations->Add(11004, ani); // Hammer Bro throw right


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

  // Cloud Platform Animations
  ani = new Animation(100); ani->Add(1300, 1000); animations->Add(1300, ani);
  ani = new Animation(100); ani->Add(1301, 1000); animations->Add(1301, ani);
  ani = new Animation(100); ani->Add(1302, 1000); animations->Add(1302, ani);
  ani = new Animation(100); ani->Add(1303, 1000); animations->Add(1303, ani);

  // Default Breakable Brick
  ani = new Animation(100); ani->Add(14, 1000); animations->Add(205, ani);

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
  AudioManager::GetInstance()->LoadSound("parry-sound",
                                         "assets/parry.MP3");
  AudioManager::GetInstance()->LoadSound("stomp", "assets/stomp.wav");
  AudioManager::GetInstance()->LoadSound("kick", "assets/kick.wav");

  // Phát nhạc intro sau khi tất cả âm thanh đã được nạp xong
  AudioManager::GetInstance()->PlayMusic("intro_theme", true);
}

void Cleanup() {
  Map::GetInstance()->Clear();

  // Dọn dẹp Scene Manager
  SceneManager::GetInstance()->Cleanup();

  HUD::DestroyInstance();
  Animations::GetInstance()->Clear();
  AudioManager::GetInstance()->CleanUp();

  Game::GetInstance()->ReleaseDirectX();
}

#pragma endregion
