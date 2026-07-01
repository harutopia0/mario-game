#include "Map.h"
#include <fstream>
#include <algorithm>
#include "../gameobject/GroundBlock.h"
#include "../gameobject/Breakable.h"

#include "../gameobject/Platform.h"
#include "../gameobject/LuckyBlock.h"
#include "../gameobject/Pipe.h"
#include "../gameobject/Goomba.h"
#include "../gameobject/Koopa.h"
#include "../gameobject/PiranhaPlant.h"
#include "../gameobject/HammerBro.h"
#include "../gameobject/Fireball.h"
#include "../gameobject/VenusFireTrap.h"
#include "../gameobject/PropSpawner.h"
#include "../gameplay/GameManager.h"
#include "../render/Camera.h"
using namespace std;

Map* Map::instance = nullptr;

Map* Map::GetInstance() {
    if (instance == nullptr) {
        instance = new Map();
    }
    return instance;
}

void Map::Clear() {
    for (GameObject* obj : objects) {
        delete obj;
    }
    objects.clear();
    for (int r = 0; r < MAX_CELL_ROW; r++) {
        for (int c = 0; c < MAX_CELL_COL; c++) {
            grid[r][c].clear();
        }
    }
}

void Map::Update(DWORD dt) {
}

void Map::Render() {
}

void Map::SpawnEnemy(float x, float y) {
    Goomba* enemy = new Goomba(x, y, GOOMBA_TYPE_NORMAL);
    objects.push_back(enemy);
    AddObjectToGrid(enemy);
}

void Map::LoadMap(LPCWSTR filePath) {
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

  if (currentLevel == 2 || currentLevel == 4) {
    animTop = 213;
    animBottom = 214;
  } else if (currentLevel == 3) {
    animTop = 217; // Cloud
    animBottom = 218;
  } else if (currentLevel == 5) {
    animTop = 250;
    animBottom = 251;
  } else {
    animTop = 211;
    animBottom = 212;
  }

  for (int r = 0; r < rows; r++) {
    Breakable* lastBrickInRow = nullptr;
    for (int c = 0; c < cols; c++) {
      int tileID = mapData[r][c];

      if (tileID != 1 && tileID != 5 && tileID != 6) {
          lastBrickInRow = nullptr;
      }

      float realX = c * 15.0f;
      float realY = ((rows - r - 1) * 15.0f) + 35.0f;

      if (tileID == 1) {
        bool hasLeft = (c > 0 && mapData[r][c - 1] == 1);
        bool hasRight = (c < cols - 1 && mapData[r][c + 1] == 1);
        bool isStair = (r < rows - 2);
        bool isTop = (r == 0 || mapData[r - 1][c] != 1 || r == rows - 2);
        bool isFloating = (r + 1 < rows && mapData[r + 1][c] != 1);

        if (currentLevel == 3 && r < 11 && !hasLeft) {
          int len = 1;
          while (c + len < cols && mapData[r][c + len] == 1) {
            len++;
          }
          if (len >= 4 && len <= 6) {
            int cloudAnimId = 1300;
            if (len == 5) cloudAnimId = 1301;
            if (len == 6) cloudAnimId = 1302;

            GroundBlock *cloud = new GroundBlock(realX, realY, cloudAnimId, len * 15.0f);
            objects.push_back(cloud);

            int cellX = (int)(realX / GRID_CELL_SIZE);
            int cellY = (int)(realY / GRID_CELL_SIZE);

            if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
                cellY < MAX_CELL_ROW) {
              AddObjectToGrid(cloud);
            }

            // Register in additional cells if the cloud spans multiple grid cells
            int endCellX = (int)((realX + len * 15.0f - 1.0f) / GRID_CELL_SIZE);
            for (int extraX = cellX + 1; extraX <= endCellX; extraX++) {
                if (extraX >= 0 && extraX < MAX_CELL_COL && cellY >= 0 && cellY < MAX_CELL_ROW) {
                    grid[cellY][extraX].push_back(cloud);
                }
            }
            c += len - 1;
            continue;
          }
        }

        int currentAnimTop = animTop;
        int currentAnimBottom = animBottom;
        
        if (currentLevel == 3 && r >= 11) {
            currentAnimTop = 211;
            currentAnimBottom = 212;
        }

        int groundAnimId = currentAnimBottom;
        if (isTop) {
          groundAnimId = currentAnimTop;
        }

        if (currentAnimTop == 211) { // 211 is Grass Top
          if (isStair) {
            if (isTop) {
              if (isFloating) {
                if (!hasLeft && !hasRight) groundAnimId = 238;
                else if (!hasLeft) groundAnimId = 236;
                else if (!hasRight) groundAnimId = 237;
                else groundAnimId = 235; // Floating Center
              } else {
                if (!hasLeft && !hasRight) groundAnimId = 228;
                else if (!hasLeft) groundAnimId = 226;
                else if (!hasRight) groundAnimId = 227;
                else groundAnimId = 225; // Top Center
              }
            } else {
              if (!hasLeft && !hasRight)
                groundAnimId = 232;
              else if (!hasLeft)
                groundAnimId = 230;
              else if (!hasRight)
                groundAnimId = 231;
              else
                groundAnimId = 229; // Mid Center
            }
          } else {
            // Normal Ground
            if (isTop) {
              if (!hasLeft && !hasRight)
                groundAnimId = 223;
              else if (!hasLeft)
                groundAnimId = 219;
              else if (!hasRight)
                groundAnimId = 220;
            } else {
              if (!hasLeft && !hasRight)
                groundAnimId = 224;
              else if (!hasLeft)
                groundAnimId = 221;
              else if (!hasRight)
                groundAnimId = 222;
            }
          }
        }

        if (!isStair || currentLevel == 3) { // Chỉ áp dụng 2 lớp cho 2 hàng dưới cùng (Mặt đất)
          GroundBlock *ground = new GroundBlock(realX, realY, groundAnimId);
          objects.push_back(ground);

          int cellX = (int)(realX / GRID_CELL_SIZE);
          int cellY = (int)(realY / GRID_CELL_SIZE);

          if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
              cellY < MAX_CELL_ROW) {
            AddObjectToGrid(ground);
          }
        } else {
          // Các khối trên không trung trở thành Gạch (Breakable) và hiển thị hình
          // Ground Block
          Breakable *brick = new Breakable(realX, realY, groundAnimId);
          objects.push_back(brick);

          int cellX = (int)(realX / GRID_CELL_SIZE);
          int cellY = (int)(realY / GRID_CELL_SIZE);

          if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
              cellY < MAX_CELL_ROW) {
            AddObjectToGrid(brick);
          }
          
          if (lastBrickInRow != nullptr) {
              lastBrickInRow->rightNeighbor = brick;
              brick->leftNeighbor = lastBrickInRow;
          }
          lastBrickInRow = brick;
          brick->SetRenderParams(isTop, isFloating, currentAnimTop);
        }

      } else if (tileID == 3) {
        Platform *platform = new Platform(realX, realY, 202);
        objects.push_back(platform);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)(realY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(platform);
        }
      } else if (tileID == 4) {
        Platform *platform = new Platform(realX, realY, 203);
        objects.push_back(platform);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)(realY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(platform);
        }
      } else if (tileID == 5) {
        int breakableAnim = 205; // Default Breakable Brick
        if (currentLevel == 1) {
          bool hasLeft = (c > 0 && (mapData[r][c - 1] == 5 || mapData[r][c - 1] == 6));
          bool hasRight = (c < cols - 1 && (mapData[r][c + 1] == 5 || mapData[r][c + 1] == 6));
          if (!hasLeft && !hasRight)
            breakableAnim = 238;
          else if (!hasLeft)
            breakableAnim = 236;
          else if (!hasRight)
            breakableAnim = 237;
          else
            breakableAnim = 235;
        } else if (currentLevel == 3) {
            breakableAnim = 1303;
        }
        Breakable *breakableBlock = new Breakable(realX, realY, breakableAnim);
        objects.push_back(breakableBlock);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)(realY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(breakableBlock);
        }

        if (lastBrickInRow != nullptr) {
            lastBrickInRow->rightNeighbor = breakableBlock;
            breakableBlock->leftNeighbor = lastBrickInRow;
        }
        lastBrickInRow = breakableBlock;
        breakableBlock->SetRenderParams(true, true, animTop);
      } else if (tileID == 6) {
        LuckyBlock *lucky = new LuckyBlock(realX, realY, 206, 201);
        objects.push_back(lucky);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)(realY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(lucky);
        }
      } else if (tileID >= 7 && tileID <= 10) {
        int pipeHeight = tileID - 5; // 7->2, 8->3, 9->4
        // Tile đánh dấu ĐỈNH ống, nhưng ống spawn từ dưới lên
        // Cần offset y xuống để chân ống sát mặt đất
        float pipeY = realY - (pipeHeight - 1) * 15.0f;
        Pipe *pipe = new Pipe(realX, pipeY, pipeHeight, false, 0, 0);
        objects.push_back(pipe);

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
        objects.push_back(goomba);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)((realY + 2.0f) / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(goomba);
        }
      } else if (tileID == 12) {
        Koopa *koopa = new Koopa(realX, realY + 2.0f, KOOPA_TYPE_GREEN);
        objects.push_back(koopa);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)((realY + 2.0f) / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(koopa);
        }
      } else if (tileID == 13) {
        Koopa *koopa = new Koopa(realX, realY + 2.0f, KOOPA_TYPE_RED);
        objects.push_back(koopa);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)((realY + 2.0f) / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(koopa);
        }
      } else if (tileID == 14) {
        Koopa *koopa = new Koopa(realX, realY + 2.0f, KOOPA_TYPE_GREEN_FLYING);
        objects.push_back(koopa);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)((realY + 2.0f) / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(koopa);
        }
      } else if (tileID == 15 || tileID == 16) {
        int pipeTile = 0;
        if (r + 1 < rows) pipeTile = mapData[r + 1][c];
        
        float plantY = realY + 16.0f; // Default (chuẩn cho ống ngắn)
        if (pipeTile >= 7 && pipeTile <= 10) {
            int pipeHeight = pipeTile - 5;
            float pipeRealY = realY - 15.0f; // Toạ độ gạch đỉnh ống
            float pipeBottomY = pipeRealY - (pipeHeight - 1) * 15.0f; // Đáy ống
            float pipeRenderHeight = (pipeHeight <= 3) ? 46.0f : ((pipeHeight - 3) * 15.0f + 46.0f);
            plantY = pipeBottomY + pipeRenderHeight; // Đỉnh ống thực sự
        }

        plantY -= 1.0f; // Dời xuống 1 pixel theo yêu cầu

        Enemy *plant = NULL;
        if (tileID == 15) {
            plant = new PiranhaPlant(realX + 8.0f, plantY);
        } else {
            plant = new VenusFireTrap(realX + 8.0f, plantY);
        }
        objects.push_back(plant);

        int cellX = (int)((realX + 8.0f) / GRID_CELL_SIZE);
        int cellY = (int)(plantY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(plant);
        }

      } else if (tileID == 18) {
        HammerBro *hammerBro = new HammerBro(realX, realY + 2.0f);
        objects.push_back(hammerBro);

        int cellX = (int)(realX / GRID_CELL_SIZE);
        int cellY = (int)(realY / GRID_CELL_SIZE);

        if (cellX >= 0 && cellX < MAX_CELL_COL && cellY >= 0 &&
            cellY < MAX_CELL_ROW) {
          AddObjectToGrid(hammerBro);
        }
      }
    }
  }
  GameManager::GetInstance()->SetMapRightEdge(cols * 15.0f);
  Camera::GetInstance()->SetMapBoundary(cols * 15.0f, rows * 15.0f);
  f.close();

  // Spawn some Jungle Props manually for visual testing
  if (currentLevel == 1) {
      PropSpawner spawner;
      spawner.SetEmptySpace(20, 100.0f, 300.0f);
      
      PropDef cliff = { 81002, 128.0f, 64.0f, 140.0f, 20, 150.0f, 350.0f };
      PropDef smallCliff = { 81003, 64.0f, 48.0f, 80.0f, 25, 100.0f, 300.0f };
      PropDef bush = { 81001, 16.0f, 16.0f, 16.0f, 35, 80.0f, 230.0f };
      
      spawner.AddProp(cliff);
      spawner.AddProp(smallCliff);
      spawner.AddProp(bush);
      
      std::vector<GameObject*> props = spawner.SpawnProps(cols, mapData, 65.0f);
      for (size_t i = 0; i < props.size(); i++) {
          objects.push_back(props[i]);
          AddObjectToGrid(props[i]);
      }

      // Mây trên trời
      PropSpawner cloudSpawner;
      cloudSpawner.SetEmptySpace(40, 50.0f, 150.0f);
      
      PropDef cloud1 = { 81004, 31.0f, 23.0f, 40.0f, 30, 50.0f, 200.0f };
      PropDef cloud2 = { 81005, 47.0f, 23.0f, 60.0f, 20, 50.0f, 200.0f };
      
      cloudSpawner.AddProp(cloud1);
      cloudSpawner.AddProp(cloud2);
      
      // Độ cao mây từ 150 (thấp) đến 220 (cao)
      std::vector<GameObject*> clouds = cloudSpawner.SpawnClouds(cols, 150.0f, 220.0f);
      for (size_t i = 0; i < clouds.size(); i++) {
          objects.push_back(clouds[i]);
          AddObjectToGrid(clouds[i]);
      }

      // Thêm 2 con rùa lăn vào cuối map để test
      Koopa *koopa1 = new Koopa(2900.0f, 150.0f, KOOPA_TYPE_GREEN);
      koopa1->Kick(1); // Lăn phải
      objects.push_back(koopa1);
      AddObjectToGrid(koopa1);

      Koopa *koopa2 = new Koopa(3000.0f, 150.0f, KOOPA_TYPE_GREEN);
      koopa2->Kick(-1); // Lăn trái
      objects.push_back(koopa2);
      AddObjectToGrid(koopa2);
  }

  if (currentLevel == 2 || currentLevel == 4) {
      PropSpawner spawner;
      // Trọng số rỗng cao hơn một chút để prop không bị quá dày đặc
      spawner.SetEmptySpace(30, 100.0f, 300.0f);
      
      // Tăng tỷ lệ (weight) và giảm khoảng cách (gap) của nấm để dễ thấy hơn
      PropDef mushroom = { 82001, 10.0f, 9.0f, 15.0f, 60, 20.0f, 80.0f };
      PropDef cliff1 = { 82002, 47.0f, 63.0f, 60.0f, 20, 100.0f, 250.0f };
      PropDef cliff2 = { 82003, 47.0f, 63.0f, 60.0f, 20, 100.0f, 250.0f };
      PropDef skull = { 82004, 47.0f, 63.0f, 60.0f, 20, 100.0f, 250.0f };
      
      spawner.AddProp(mushroom);
      spawner.AddProp(cliff1);
      spawner.AddProp(cliff2);
      spawner.AddProp(skull);
      
      float groundY = (currentLevel == 4) ? 65.0f : 50.0f; // Level 4 ground ở r=13, Level 2 ground ở r=14
      
      std::vector<GameObject*> props = spawner.SpawnProps(cols, mapData, groundY);
      for (size_t i = 0; i < props.size(); i++) {
          objects.push_back(props[i]);
          AddObjectToGrid(props[i]);
      }
      
      // Bầu trời: Mây nhỏ (sao) cho màn 2 và 4
      PropSpawner cloudSpawner;
      cloudSpawner.SetEmptySpace(20, 30.0f, 100.0f); // Dày hơn mây bình thường một chút
      
      PropDef smallCloud = { 82005, 8.0f, 11.0f, 15.0f, 80, 20.0f, 60.0f };
      cloudSpawner.AddProp(smallCloud);
      
      std::vector<GameObject*> skyClouds = cloudSpawner.SpawnClouds(cols, 120.0f, 220.0f);
      for (size_t i = 0; i < skyClouds.size(); i++) {
          objects.push_back(skyClouds[i]);
          AddObjectToGrid(skyClouds[i]);
      }
  }

  if (currentLevel == 3) {
      PropSpawner cloudSpawner;
      // Giảm khoảng trống (weight rỗng = 5, gap chỉ từ 10-30) để mây dày đặc hơn
      cloudSpawner.SetEmptySpace(5, 10.0f, 30.0f);
      
      // Tăng tỷ lệ mây đơn (nhỏ) và mây đôi (lớn), giảm minGap/maxGap để chúng kết hợp sát nhau
      PropDef cloud1 = { 81004, 31.0f, 23.0f, 10.0f, 50, 5.0f, 20.0f };
      PropDef cloud2 = { 81005, 47.0f, 23.0f, 10.0f, 50, 5.0f, 20.0f };
      
      cloudSpawner.AddProp(cloud1);
      cloudSpawner.AddProp(cloud2);
      
      // Spawn dải mây trên không (200 - 250)
      std::vector<GameObject*> skyClouds = cloudSpawner.SpawnClouds(cols, 200.0f, 250.0f);
      for (size_t i = 0; i < skyClouds.size(); i++) {
          objects.push_back(skyClouds[i]);
          AddObjectToGrid(skyClouds[i]);
      }
      
      // Spawn dải mây dưới ground (0 - 60)
      std::vector<GameObject*> groundClouds = cloudSpawner.SpawnClouds(cols, 0.0f, 60.0f);
      for (size_t i = 0; i < groundClouds.size(); i++) {
          objects.push_back(groundClouds[i]);
          AddObjectToGrid(groundClouds[i]);
      }
  }
}


void Map::AddObjectToGrid(GameObject *obj) {
  int col = (int)(obj->GetX() / GRID_CELL_SIZE);
  int row = (int)(obj->GetY() / GRID_CELL_SIZE);
  if (col < 0 || col >= MAX_CELL_COL || row < 0 || row >= MAX_CELL_ROW)
    return;
  grid[row][col].push_back(obj);

  obj->gridRow = row;
  obj->gridCol = col;
}

void Map::RemoveObjectFromGrid(GameObject *obj) {
  if (obj->gridRow < 0 || obj->gridCol < 0)
    return;
  auto &cell = grid[obj->gridRow][obj->gridCol];
  cell.erase(std::remove(cell.begin(), cell.end(), obj), cell.end());
}

void Map::UpdateObjectGrid(GameObject *obj) {
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
