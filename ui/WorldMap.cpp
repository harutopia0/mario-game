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
    switch (level)
    {
    case 1: SetSelectedNode(1); break;
    case 2: SetSelectedNode(2); break;
    case 3: SetSelectedNode(3); break;
    case 4: SetSelectedNode(4); break;
    case 5: SetSelectedNode(6); break;
    default: SetSelectedNode(0); break;
    }
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

    if (fromNode <= 5 && toNode <= 5)
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

        if (targetNode == 6 || currentNode == 6)
        {
            currentAnimID = (facingDir > 0) ? 104 : 105;
        }
        else
        {
            currentAnimID = (facingDir > 0) ? 102 : 103;
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

        // Tọa độ vẽ tạm thời để tính toán bù trừ sai lệch pixel
        float drawX = marioX;
        float drawY = marioY;

        // XỬ LÝ BÙ TRỪ KHI QUAY TRÁI
        if (facingDir == -1)
        {
            // Thay đổi các giá trị cộng/trừ dưới đây cho đến khi khớp mắt nhất
            drawX += -3.0f;  // Dịch sang phải 3 pixel (hoặc đổi thành trừ nếu bị lệch ngược lại)
            drawY += 0.0f;  // Tương tự cho trục đứng nếu cần sửa
        }

        D3DXMatrixScaling(&matScale, scaleFactor, scaleFactor, 1.0f);
        game->GetSpriteHandler()->SetViewTransform(&matScale);

        // Sử dụng tọa độ vẽ đã qua tinh chỉnh
        ani->Render(drawX / scaleFactor, drawY / scaleFactor);

        D3DXMatrixScaling(&matScale, 1.0f, 1.0f, 1.0f);
        game->GetSpriteHandler()->SetViewTransform(&matScale);
    }
}