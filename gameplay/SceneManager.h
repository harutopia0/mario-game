#pragma once
#include <Windows.h>

// Thời gian tạm dừng khi Mario biến lớn (mili-giây)
#define MARIO_TRANSFORM_PAUSE_TIME 1000

#define GRID_CELL_SIZE 64.0f
#define MAX_CELL_ROW 50
#define MAX_CELL_COL 200

enum GameState {
    STATE_INTRO,
    STATE_WORLD_MAP,
    STATE_PLAYING
};

class Intro;
class WorldMap;

class SceneManager {
private:
    static SceneManager* instance;
    GameState currentState;
    Intro* introScene;
    WorldMap* worldMapScene;

    // Quản lý đếm ngược thời gian chết tập trung
    bool isMarioDying;
    DWORD deathStartTime;

    // Quản lý đếm ngược thời gian qua màn thường
    bool isMarioLevelClearing;
    DWORD levelClearStartTime;

    // Quản lý đếm ngược thời gian thắng màn cuối (Phá đảo)
    bool isMarioGameWinning;

    // Quản lý tạm dừng khi Mario biến lớn
    bool isMarioTransforming;
    DWORD gameWinStartTime;
    DWORD transformStartTime;

    bool isMarioCastingSkill;
    DWORD castSkillStartTime;

    // Cơ chế Roulette Thẻ bài khi thắng màn
    int rouletteCardType;
    DWORD lastRouletteTick;
    bool isRouletteDone;

    SceneManager();
public:
    static SceneManager* GetInstance();
    void Init();
    void Update(DWORD dt);
    void Render();
    void Cleanup();

    void ProcessMarioCastSkill(int cardType, int slot);
    bool IsCastingSkill() const { return isMarioCastingSkill; }

    GameState GetState() const { return currentState; }
    void SwitchTo(GameState newState);
    void ProcessMarioDeath(); // Hàm đếm ngược 5s khi Mario chết
    void ProcessLevelClear(); // Hàm xử lý qua màn thường 6s
    void ProcessGameWin();    // Hàm xử lý thắng game màn cuối 6s
    void ProcessTransform();  // Hàm tạm dừng khi Mario biến lớn
    bool IsTransforming() const { return isMarioTransforming; }
};