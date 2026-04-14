#include "Mario.h"
#include "../animation/Animations.h"

Mario::Mario(float x, float y) : GameObject(x, y) {}

void Mario::Update(DWORD dt)
{
    float speed = 0.3f;
    vx = 0.0f;

    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        vx = speed;
        nx = 1;
    }
    else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
        vx = -speed;
        nx = -1;
    }

    if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && y == 48.0f)
    {
        vy = MARIO_JUMP_SPEED_Y;
    }

    vy += MARIO_GRAVITY * dt;

    GameObject::Update(dt);

    if (y <= 48.0f)
    {
        y = 48.0f;
        vy = 0.0f;
    }
}

void Mario::Render()
{
    Animation* ani = NULL;

    if (y > 48.0f) 
    {
        if (nx > 0) ani = Animations::GetInstance()->Get(104);
        else ani = Animations::GetInstance()->Get(105);
    }
    else 
    {
        // Đứng yên (vx == 0)
        if (vx == 0.0f) 
        {
            if (nx > 0) ani = Animations::GetInstance()->Get(100);
            else ani = Animations::GetInstance()->Get(101);
        }
        else 
        {
            if (nx > 0) ani = Animations::GetInstance()->Get(102);
            else ani = Animations::GetInstance()->Get(103);
        }
    }

    if (ani != NULL) ani->Render(x, y);
}