#pragma once
#include <Windows.h>

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

    SceneManager();
public:
    static SceneManager* GetInstance();
    void Init();
    void Update(DWORD dt);
    void Render();
    void Cleanup();

    GameState GetState() const { return currentState; }
    void SwitchTo(GameState newState);
    void ProcessMarioDeath(); // Hàm đếm ngược 5s khi Mario chết
};