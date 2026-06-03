#pragma once
#include <Windows.h>
#include "../render/Sprites.h"

class WorldMap
{
private:
    bool isDone;
    int selectedLevel; // Màn chơi được chọn (1, 2, 3...)
    float marioX, marioY; // Tọa độ icon Mario trên map

public:
    WorldMap();
    void LoadSprites();
    void Update(DWORD dt);
    void Render();

    bool IsDone() { return isDone; }
    int GetSelectedLevel() { return selectedLevel; }
};