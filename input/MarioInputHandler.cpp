#include "MarioInputHandler.h"
#include "../gameobject/Mario.h"

MarioInputHandler::MarioInputHandler(Mario* mario)
{
    this->mario = mario;
}

void MarioInputHandler::KeyState(BYTE* state)
{
    if (mario->IsDied() || mario->isEnteringPipe)
        return;

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
}

void MarioInputHandler::OnKeyDown(int KeyCode)
{
    // If you hook this up to a system that processes WM_KEYDOWN, you can handle keys here.
    // For now, we process Jump inside KeyState.
}

void MarioInputHandler::OnKeyUp(int KeyCode)
{
}
