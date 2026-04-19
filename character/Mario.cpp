#include "Mario.h"
#include "../gameplay/Brick.h"
#include "../animation/Animations.h"
#include <algorithm>

#define MARIO_JUMP_SPEED_Y 0.5f
#define MARIO_GRAVITY -0.002f

Mario::Mario(float x, float y) : GameObject(x, y) {}

void Mario::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + 42.0f;
    bottom = y + 48.0f;
}

void Mario::Update(DWORD dt, vector<GameObject*>* coObjects)
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

    if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && vy == 0.0f)
    {
        vy = MARIO_JUMP_SPEED_Y;
    }

    vy += MARIO_GRAVITY * dt;

    float dx = vx * dt;
    float dy = vy * dt;

    float min_tx = 1.0f, min_ty = 1.0f;
    float nx_col = 0, ny_col = 0;

    float ml, mt, mr, mb;
    GetBoundingBox(ml, mt, mr, mb);

    for (UINT i = 0; i < coObjects->size(); i++)
    {
        GameObject* e = coObjects->at(i);
        if (e == this) continue;

        if (dynamic_cast<Brick*>(e))
        {
            float sl, st, sr, sb;
            e->GetBoundingBox(sl, st, sr, sb);

            float t, temp_nx, temp_ny;
            SweptAABB(ml, mt, mr, mb, dx, dy, sl, st, sr, sb, t, temp_nx, temp_ny);

            if (t < 1.0f)
            {
                if (temp_ny != 0) { min_ty = min(min_ty, t); ny_col = temp_ny; }
                if (temp_nx != 0) { min_tx = min(min_tx, t); nx_col = temp_nx; }
            }
        }
    }

    x += min_tx * dx + nx_col * 0.4f;
    y += min_ty * dy + ny_col * 0.4f;

    if (ny_col != 0) vy = 0.0f;
    if (nx_col != 0) vx = 0.0f;
}

void Mario::Render()
{
    Animation* ani = NULL;

    if (vy != 0.0f)
    {
        if (nx > 0) ani = Animations::GetInstance()->Get(104);
        else ani = Animations::GetInstance()->Get(105);
    }
    else
    {
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