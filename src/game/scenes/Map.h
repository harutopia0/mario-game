#pragma once
#include "engine/core/GameObject.h"
#include "game/entities/Mario.h"
#include "game/scenes/SceneManager.h"
#include <Windows.h>
#include <string>
#include <vector>

class Map
{
  private:
    static Map *instance;
    std::vector<GameObject *> objects;
    std::vector<GameObject *> grid[MAX_CELL_ROW][MAX_CELL_COL];

    Map()
    {
    }

  public:
    static Map *GetInstance();

    void LoadMap(LPCWSTR filePath);
    void Update(DWORD dt);
    void Render();
    void Clear();

    void AddObjectToGrid(GameObject *obj);
    void RemoveObjectFromGrid(GameObject *obj);
    void UpdateObjectGrid(GameObject *obj);
    void SpawnEnemy(float x, float y);

    std::vector<GameObject *> &GetObjects()
    {
        return objects;
    }

    using GridType = std::vector<GameObject *>[MAX_CELL_ROW][MAX_CELL_COL];
    GridType &GetGrid()
    {
        return grid;
    }

    Mario *GetMario()
    {
        if (objects.empty())
            return nullptr;
        return dynamic_cast<Mario *>(objects[0]);
    }
};
