#include "Enemy.h"
#include "../gameobject/Brick.h"
#include "../physics/Collision.h"


#include "../animation/Animations.h"

Enemy::Enemy(float x, float y, float width, float height)
    : GameObject(x, y)
{
    this->width = width;
    this->height = height;

    vx = 0.0f;
    vy = 0.0f;

    died = false;
}

void Enemy::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void Enemy::Update(DWORD dt, vector<GameObject*>* coObjects)
{
    if (died) return;

    // Gravity
    vy += ENEMY_GRAVITY * dt;

    float dx = vx * dt;
    float dy = vy * dt;

    // Move X trước
    x += dx;

    float ml, mt, mr, mb;
    GetBoundingBox(ml, mt, mr, mb);

    float min_ty = 1.0f;
    float ny_col = 0;

    // Check collision với brick
    for (GameObject* obj : *coObjects)
    {
        if (obj == this) continue;
        Brick* brick = dynamic_cast<Brick*>(obj);
        if (brick)
        {
            float sl, st, sr, sb;
            brick->GetBoundingBox(sl, st, sr, sb);
            //Chỉ check nếu overlap X
            if (mr > sl && ml < sr)
            {
                float t, nx, ny;
                Collision::GetInstance()->SweptAABB(
                    ml, mt, mr, mb,
                    0, dy,
                    sl, st, sr, sb,
                    t, nx, ny
                );
                if (t < min_ty && ny != 0)
                {
                    min_ty = t;
                    ny_col = ny;
                }
            }
        }
    }
    y += min_ty * dy + ny_col * 0.1f;
    //khi chạm đất
    if (ny_col != 0)
    {
        vy = 0;
    }
}

void Enemy::Render()
{
    if (died) return;
    Animations::GetInstance()->Get(300)->Render(x, y);
}

void Enemy::OnCollision(GameObject* obj)
{
}