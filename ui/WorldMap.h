#pragma once
#include <Windows.h>
#include "../render/Sprites.h"

class WorldMap
{
private:
    bool isDone;
    int currentNode;     // Node hiện tại đang đứng yên (0->6)
    int targetNode;      // Node đích đang di chuyển tới
    bool isMoving;       // Trạng thái kiểm soát đang di chuyển
    float moveSpeed;     // Vận tốc di chuyển trên bản đồ

    float marioX, marioY;       // Tọa độ thực tế của Mario hiện tại
    float targetX, targetY;     // Tọa độ đích cần đến từng chặng

    // Hệ thống mốc di chuyển phân đoạn (Waypoints) để bám lòng đường đất
    float wpX[3];
    float wpY[3];
    int numWaypoints;
    int currentWpIndex;

    int facingDir;       // Hướng nhìn của Mario (1: Phải, -1: Trái)
    int currentAnimID;   // ID Animation hiện tại để vẽ

    void GetNodePosition(int node, float& ox, float& oy);
    void UpdateMarioPosition();
    void InitiateMove(int fromNode, int toNode); // Hàm xử lý tính toán lộ trình đường đi
    bool IsNodeLocked(int node);                 // Kiểm tra node có bị khoá không

public:
    WorldMap();
    void LoadSprites();
    void Update(DWORD dt);
    void Render();

    bool IsDone() { return isDone; }
    int GetSelectedLevel();
    int GetCurrentNode() { return currentNode; }
    void SetSelectedNode(int node);
    void SetLevelNode(int level);
    void Reset() { isDone = false; }
};