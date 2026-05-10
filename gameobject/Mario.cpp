#include "Mario.h"
#include "../gameobject/Brick.h"
#include "../gameobject/Flag.h"
#include "../gameobject/Enemy.h"
#include "../gameobject/Buff.h"
#include "../animation/Animations.h"
#include "../gameplay/GameManager.h"
#include "../physics/Collision.h"
#include <algorithm>

#define MARIO_JUMP_SPEED_Y      0.27f 
#define MARIO_GRAVITY           -0.00067f
#define MARIO_WALKING_SPEED		0.15f
#define MARIO_ACCEL_WALK_X		0.0005f 
#define MARIO_FRICTION			0.0004f

Mario::Mario(float x, float y, float width, float height) : GameObject(x, y), width(width), height(height)
{
    isOnGround = false;
    ax = 0.0f;
}

void Mario::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    float realHitboxWidth = 13.0f;
    float offsetX = (this->width - realHitboxWidth) / 2.0f;

    left = x + offsetX;
    top = y;
    right = left + realHitboxWidth;
    bottom = y + this->height;
}

void Mario::Update(DWORD dt, vector<GameObject*>* coObjects)
{
    // PHẦN VẬT LÝ DI CHUYỂN
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        ax = MARIO_ACCEL_WALK_X;
        nx = 1;
    }
    else if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        ax = -MARIO_ACCEL_WALK_X;
        nx = -1;
    }
    else {
        ax = 0.0f;
    }

    if (ax == 0.0f) {
        if (vx > 0) {
            vx -= MARIO_FRICTION * dt;
            if (vx < 0) vx = 0.0f;
        }
        else if (vx < 0) {
            vx += MARIO_FRICTION * dt;
            if (vx > 0) vx = 0.0f;
        }
    }
    vx += ax * dt;

    if (vx > MARIO_WALKING_SPEED) vx = MARIO_WALKING_SPEED;
    if (vx < -MARIO_WALKING_SPEED) vx = -MARIO_WALKING_SPEED;

    if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && isOnGround) {
        vy = MARIO_JUMP_SPEED_Y;
        isOnGround = false;
    }

    vy += MARIO_GRAVITY * dt;

    float dx = vx * dt;
    float dy = vy * dt;

    // QUÉT VA CHẠM TRỤC X (ĐI NGANG)
    float min_tx = 1.0f;
    float nx_col = 0;
    float ml, mt, mr, mb;
    GetBoundingBox(ml, mt, mr, mb);

    for (UINT i = 0; i < coObjects->size(); i++)
    {
        GameObject* e = coObjects->at(i);
        if (e == this) continue;

        float sl, st, sr, sb;
        e->GetBoundingBox(sl, st, sr, sb);

        if (mb > st && mt < sb)
        {
            float t, temp_nx, temp_ny;
            Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, dx, 0.0f, sl, st, sr, sb, t, temp_nx, temp_ny);

            if (t < 1.0f && temp_nx != 0)
            {
                // 1. ĐỤNG GẠCH (Cản đường lại)
                if (dynamic_cast<Brick*>(e)) {
                    if (t < min_tx) {
                        min_tx = t;
                        nx_col = temp_nx;
                    }
                }
                // 2. ĐỤNG QUÁI VẬT (Bị thương)
                else if (Enemy* enemy = dynamic_cast<Enemy*>(e)) {
                    OutputDebugStringA("Mario collided with enemy\n");
                   
                }
                // 3. ĐỤNG BUFF
                else if (Buff* buff = dynamic_cast<Buff*>(e)) {

                }
                // 4. CHẠM CỜ
                else if (Flag* flag = dynamic_cast<Flag*>(e)) {

                }
            }
        }
    }

    x += min_tx * dx + nx_col * 0.01f;
    if (nx_col != 0) vx = 0.0f;

    // QUÉT VA CHẠM TRỤC Y (RƠI / NHẢY)
    GetBoundingBox(ml, mt, mr, mb);
    float min_ty = 1.0f;
    float ny_col = 0;

    for (UINT i = 0; i < coObjects->size(); i++)
    {
        GameObject* e = coObjects->at(i);
        if (e == this) continue;

        float sl, st, sr, sb;
        e->GetBoundingBox(sl, st, sr, sb);

        if (mr > sl && ml < sr)
        {
            float t, temp_nx, temp_ny;
            Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, 0.0f, dy, sl, st, sr, sb, t, temp_nx, temp_ny);

            if (t < 1.0f && temp_ny != 0)
            {
                // 1. ĐỤNG GẠCH (Cản cả trên lẫn dưới)
                if (dynamic_cast<Brick*>(e)) {
                    if (t < min_ty) {
                        min_ty = t;
                        ny_col = temp_ny;
                    }
                }

                // 2. ĐỤNG NỀN TẢNG 1 CHIỀU (Chỉ cản khi rơi từ trên xuống)
                else if (dynamic_cast<Platform*>(e)) {
                    if (temp_ny == 1) {
                        if (t < min_ty) {
                            min_ty = t;
                            ny_col = temp_ny;
                        }
                    }
                }
                // 3. ĐỤNG QUÁI VẬT TRỤC DỌC
                else if (Enemy* enemy = dynamic_cast<Enemy*>(e)) {
                    if (!enemy->IsDied()) {
                        if (temp_ny == 1) {
                            // Rơi xuống đụng enemy
                            vy = MARIO_JUMP_SPEED_Y * 0.5f; // Nảy nhẹ lên
                            OutputDebugStringA("Enemy stomped!\n");
                            enemy->SetDied(true);
                        }
                        else if (temp_ny == -1) {
                            OutputDebugStringA("Mario damaged by enemy\n");
                            // Nhảy lên đụng enemy
                            lives--;

                            if (lives <= 0)
                            {
                                SetDied(true);
                            }
                        }
                    }
                }
            }
        }
    }

    y += min_ty * dy + ny_col * 0.01f;

    if (ny_col != 0)
    {
        vy = 0.0f;
        if (ny_col == 1) isOnGround = true;
    }
    else
    {
        isOnGround = false;
    }

    // XỬ LÝ GAME OVER
    if (IsDied())
    {
        GameManager::GetInstance()->SetGameOver(true);
        std::printf("game over\n");
    }
}

void Mario::Render()
{
    Animation* ani = NULL;
    bool isSkidding = (vx > 0 && nx < 0) || (vx < 0 && nx > 0);

    if (!isOnGround)
    {
        if (nx > 0) ani = Animations::GetInstance()->Get(104);
        else ani = Animations::GetInstance()->Get(105);
    }
    else
    {
        if (isSkidding)
        {
            if (nx > 0) ani = Animations::GetInstance()->Get(107);
            else ani = Animations::GetInstance()->Get(106);
        }
        else if (vx == 0.0f)
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

void Mario::OnCollision(GameObject* obj)
{
    if (Flag* flag = dynamic_cast<Flag*>(obj))
    {
        if (!flag->GetVisited())
        {
            flag->SetVisited();
        }
    }

    else if (Enemy* enemy = dynamic_cast<Enemy*>(obj))
    {
        if (!enemy->IsDied())
        {
            lives--;
            if (lives <= 0)
            {
                SetDied(true);
            }
        }
    }

    else if (Buff* buff = dynamic_cast<Buff*>(obj))
    {
        lives++;
		//Destroy buff after collecting
    }
}