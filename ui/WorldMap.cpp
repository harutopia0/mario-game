#include "WorldMap.h"
#include "../render/Sprites.h"
#include "../animation/Animations.h"
#include "../core/Game.h"
#include <cmath>

WorldMap::WorldMap()
{
    isDone = false;
    currentNode = 0;
    targetNode = 0;
    isMoving = false;
    moveSpeed = 0.12f;
    facingDir = 1;
    currentAnimID = 100;
    numWaypoints = 0;
    currentWpIndex = 0;
    UpdateMarioPosition();
}

void WorldMap::LoadSprites()
{
}

void WorldMap::GetNodePosition(int node, float& ox, float& oy)
{
    switch (node)
    {
    case 0: ox = 44.0f;  oy = 73.0f; break;  // Ô đứng tạm 1
    case 1: ox = 107.0f; oy = 93.0f; break;  // Level 1
    case 2: ox = 218.0f; oy = 93.0f; break;  // Level 2
    case 3: ox = 329.0f; oy = 93.0f; break;  // Level 3
    case 4: ox = 442.0f; oy = 93.0f; break;  // Level 4
    case 5: ox = 541.0f; oy = 73.0f; break;  // Ô đứng tạm 2
    case 6: ox = 545.0f; oy = 189.0f; break; // Level 5
    default: break;
    }
}

void WorldMap::UpdateMarioPosition()
{
    GetNodePosition(currentNode, marioX, marioY);
    targetX = marioX;
    targetY = marioY;
}

int WorldMap::GetSelectedLevel()
{
    if (isMoving) return 0;

    switch (currentNode)
    {
    case 1: return 1;
    case 2: return 2;
    case 3: return 3;
    case 4: return 4;
    case 6: return 5;
    default: return 0;
    }
}

void WorldMap::SetSelectedNode(int node)
{
    if (node >= 0 && node <= 6)
    {
        currentNode = node;
        targetNode = node;
        isMoving = false;
        UpdateMarioPosition();
    }
}

void WorldMap::SetLevelNode(int level)
{
    int tNode = 0;
    switch (level)
    {
    case 1: tNode = 1; break;
    case 2: tNode = 2; break;
    case 3: tNode = 3; break;
    case 4: tNode = 4; break;
    case 5: tNode = 6; break;
    default: tNode = 0; break;
    }

    InitiateMove(currentNode, tNode);
}

void WorldMap::InitiateMove(int fromNode, int toNode)
{
    targetNode = toNode;
    float startX, startY, endX, endY;
    GetNodePosition(fromNode, startX, startY);
    GetNodePosition(toNode, endX, endY);

    numWaypoints = 0;
    currentWpIndex = 0;

    if (endX > startX) facingDir = 1;
    else if (endX < startX) facingDir = -1;

    // Lộ trình tự động khi thắng Màn 4 -> Chạy qua Ô tạm 2 -> Lên Màn 5
    if (fromNode == 4 && toNode == 6)
    {
        float pathY = 73.0f;
        float node5X = 541.0f;
        float node5Y = 73.0f;

        wpX[0] = startX;  wpY[0] = pathY;  // Chặng 0: Từ Level 4 xuống đường mòn
        wpX[1] = node5X;  wpY[1] = node5Y;  // Chặng 1: Chạy bộ ngang qua Ô tạm 2
        wpX[2] = endX;    wpY[2] = endY;    // Chặng 2: Từ Ô tạm 2 nhảy thẳng lên Level 5

        numWaypoints = 3;
    }
    else if (fromNode <= 5 && toNode <= 5)
    {
        float pathY = 73.0f;

        wpX[0] = startX; wpY[0] = pathY;
        wpX[1] = endX;   wpY[1] = pathY;
        wpX[2] = endX;   wpY[2] = endY;

        numWaypoints = 3;
    }
    else
    {
        wpX[0] = endX;
        wpY[0] = endY;
        numWaypoints = 1;
    }

    targetX = wpX[0];
    targetY = wpY[0];
    isMoving = true;
}

void WorldMap::Update(DWORD dt)
{
    if (isMoving)
    {
        float dx = targetX - marioX;
        float dy = targetY - marioY;
        float dist = sqrt(dx * dx + dy * dy);
        float step = moveSpeed * dt;

        if (dist <= step)
        {
            marioX = targetX;
            marioY = targetY;

            currentWpIndex++;
            if (currentWpIndex < numWaypoints)
            {
                targetX = wpX[currentWpIndex];
                targetY = wpY[currentWpIndex];
            }
            else
            {
                isMoving = false;
                currentNode = targetNode;
            }
        }
        else
        {
            marioX += (dx / dist) * step;
            marioY += (dy / dist) * step;
        }

        // ĐÃ SỬA LOGIC ANIMATION: Tách biệt trạng thái chạy bộ và nhảy cao
        currentAnimID = (facingDir > 0) ? 102 : 103; // Mặc định trên đường là Chạy bộ

        if (targetNode == 6)
        {
            // Nếu đi từ Màn 4 -> Màn 5 (3 chặng), chỉ chặng cuối cùng (index == 2) mới hóa Sprite Nhảy
            if (numWaypoints == 3 && currentWpIndex == 2)
            {
                currentAnimID = (facingDir > 0) ? 104 : 105;
            }
            // Nếu người chơi chủ động bấm LÊN từ Ô tạm 2 -> Màn 5 (1 chặng) thì dùng Sprite Nhảy luôn
            else if (numWaypoints == 1)
            {
                currentAnimID = (facingDir > 0) ? 104 : 105;
            }
        }
        else if (currentNode == 6 && targetNode == 5)
        {
            // Người chơi chủ động bấm XUỐNG từ Màn 5 -> Ô tạm 2
            currentAnimID = (facingDir > 0) ? 104 : 105;
        }
        return;
    }

    currentAnimID = (facingDir > 0) ? 100 : 101;

    static bool isEnterPressed = true;
    if ((GetAsyncKeyState(VK_RETURN) & 0x8000) || (GetAsyncKeyState(VK_SPACE) & 0x8000))
    {
        if (!isEnterPressed && GetSelectedLevel() > 0)
        {
            isDone = true;
            isEnterPressed = true;
        }
    }
    else isEnterPressed = false;

    static bool isRightPressed = false;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        if (!isRightPressed && currentNode >= 0 && currentNode <= 4) {
            InitiateMove(currentNode, currentNode + 1);
            isRightPressed = true;
        }
    }
    else isRightPressed = false;

    static bool isLeftPressed = false;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
        if (!isLeftPressed && currentNode >= 1 && currentNode <= 5) {
            InitiateMove(currentNode, currentNode - 1);
            isLeftPressed = true;
        }
    }
    else isLeftPressed = false;

    static bool isUpPressed = false;
    if (GetAsyncKeyState(VK_UP) & 0x8000)
    {
        if (!isUpPressed && currentNode == 5) {
            InitiateMove(currentNode, 6);
            isUpPressed = true;
        }
    }
    else isUpPressed = false;

    static bool isDownPressed = false;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000)
    {
        if (!isDownPressed && currentNode == 6) {
            InitiateMove(currentNode, 5);
            isDownPressed = true;
        }
    }
    else isDownPressed = false;
}

void WorldMap::Render()
{
    Sprites* sprites = Sprites::GetInstance();

    if (sprites->Get(8000))
    {
        sprites->Get(8000)->Draw(0.0f, 0.0f);
    }

    Animation* ani = Animations::GetInstance()->Get(currentAnimID);
    if (ani != NULL)
    {
        Game* game = Game::GetInstance();
        D3DXMATRIX matScale;
        float scaleFactor = 2.0f;

        float drawX = marioX;
        float drawY = marioY;

        if (facingDir == -1)
        {
            drawX += -3.0f;
            drawY += 0.0f;
        }

        D3DXMatrixScaling(&matScale, scaleFactor, scaleFactor, 1.0f);
        game->GetSpriteHandler()->SetViewTransform(&matScale);

        ani->Render(drawX / scaleFactor, drawY / scaleFactor);

        D3DXMatrixScaling(&matScale, 1.0f, 1.0f, 1.0f);
        game->GetSpriteHandler()->SetViewTransform(&matScale);
    }
}