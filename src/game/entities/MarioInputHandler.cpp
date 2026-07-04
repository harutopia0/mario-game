#include "game/entities/MarioInputHandler.h"
#include "game/entities/Mario.h"
#include "game/scenes/GameManager.h"

MarioInputHandler::MarioInputHandler(Mario *mario)
{
    this->mario = mario;
}

void MarioInputHandler::KeyState(BYTE *state)
{
    bool isControlLocked = GameManager::GetInstance()->IsLevelClear() ||
                           GameManager::GetInstance()->IsGameWin();
    if (isControlLocked)
    {
        mario->SetAccelX(0.0f);
        return;
    }

    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        mario->SetAccelX(MARIO_ACCEL_WALK_X);
        mario->SetDirection(1);
    }
    else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
        mario->SetAccelX(-MARIO_ACCEL_WALK_X);
        mario->SetDirection(-1);
    }
    else
    {
        mario->SetAccelX(0.0f);
    }

    if (GetAsyncKeyState(VK_DOWN) & 0x8000)
    {
        mario->SetPressingDown(true);
    }
    else
    {
        mario->SetPressingDown(false);
    }

    // Xử lý nhảy: giữ Space = nhảy cao, nhả sớm = nhảy thấp
    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    {
        mario->Jump();
        mario->SetHoldingJump(true);
    }
    else
    {
        mario->SetHoldingJump(false);
    }

    // Bắn lửa / chém: phím Z
    static bool isZPressed = false;
    if (GetAsyncKeyState('Z') & 0x8000)
    {
        if (!isZPressed)
        {
            if (mario->IsScissors())
            {
                mario->StartParry();
            }
            else
            {
                mario->ShootFireball();
            }
            isZPressed = true;
        }
    }
    else
    {
        isZPressed = false;
    }
    // Xử lý tự sát / nhận sát thương: phím R
    static bool isRPressed = false;
    if (GetAsyncKeyState('R') & 0x8000)
    {
        if (!isRPressed)
        {
            mario->TakeDamage();
            isRPressed = true;
        }
    }
    else
    {
        isRPressed = false;
    }
}
