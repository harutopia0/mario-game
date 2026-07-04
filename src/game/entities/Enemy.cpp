#include "game/entities/Enemy.h"
#include "game/objects/Block.h"
#include "game/objects/Platform.h"
#include "engine/core/Collision.h"
#include "engine/graphics/Animations.h"

Enemy::Enemy(float x, float y, int animationId)
    : GameObject(x, y)
{
    this->animationId = animationId;
    this->layer = LAYER_ENEMIES;
    Animation* anim = Animations::GetInstance()->Get(animationId);
    if (anim != NULL) {
        this->width = anim->GetWidth();
        this->height = anim->GetHeight();
    } else {
        this->width = 16;
        this->height = 16;
    }
    vx = 0.0f;
    vy = 0.0f;
    died = false;
    isFreezed = false;
    layer = LAYER_ENEMIES;
    nx = -1; // Mặc định hướng sang trái
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
    if (isFreezed) {
        vx = 0.0f;
        vy = 0.0f;
        return;
    }

    // Trọng lực
    vy += ENEMY_GRAVITY * dt;

    float dx = vx * dt;
    float dy = vy * dt;

    float ml, mt, mr, mb;
    GetBoundingBox(ml, mt, mr, mb);

    // 1. Kiểm tra va chạm trục X
    float min_tx = 1.0f;
    float nx_col = 0;

    for (GameObject* obj : *coObjects)
    {
        if (obj == this || obj->IsDeleted()) continue;
        Block* block = dynamic_cast<Block*>(obj);
        if (block && !block->IsOneWay()) // Không va chạm ngang với Platform
        {
            float sl, st, sr, sb;
            block->GetBoundingBox(sl, st, sr, sb);
            if (mb > st && mt < sb)
            {
                float t, temp_nx, temp_ny;
                Collision::GetInstance()->SweptAABB(
                    ml, mt, mr, mb,
                    dx, 0.0f,
                    sl, st, sr, sb,
                    t, temp_nx, temp_ny
                );
                if (t < min_tx && temp_nx != 0)
                {
                    min_tx = t;
                    nx_col = temp_nx;
                }
            }
        }
    }

    x += min_tx * dx + nx_col * 0.01f;
    if (nx_col != 0)
    {
        vx = -vx; // Đảo chiều vận tốc
        nx = -nx; // Đảo hướng vẽ
    }

    // Cập nhật lại bounding box sau khi đã di chuyển X
    GetBoundingBox(ml, mt, mr, mb);

    // 2. Kiểm tra va chạm trục Y
    float min_ty = 1.0f;
    float ny_col = 0;

    for (GameObject* obj : *coObjects)
    {
        if (obj == this || obj->IsDeleted()) continue;
        Block* block = dynamic_cast<Block*>(obj);
        if (block)
        {
            float sl, st, sr, sb;
            block->GetBoundingBox(sl, st, sr, sb);
            if (mr > sl && ml < sr)
            {
                float t, temp_nx, temp_ny;
                Collision::GetInstance()->SweptAABB(
                    ml, mt, mr, mb,
                    0.0f, dy,
                    sl, st, sr, sb,
                    t, temp_nx, temp_ny
                );
                if (t < min_ty && temp_ny != 0)
                {
                    min_ty = t;
                    ny_col = temp_ny;
                }
            }
        }
    }

    y += min_ty * dy + ny_col * 0.01f;
    bool isOnGround = false;
    if (ny_col != 0)
    {
        vy = 0;
        if (ny_col == 1) isOnGround = true;
    }

    // Xử lý rớt khỏi map
    if (y < 0.0f) {
        this->died = true;
        this->isDeleted = true;
        return;
    }

    // Ledge Detection (Quay đầu khi đến mép vực)
    if (isOnGround && !CanFallOffLedge())
    {
        bool hasGroundAhead = false;
        
        // Tạo một bounding box nhỏ (feeler) phía trước và ngay dưới chân enemy
        float feelerL = (vx > 0) ? (x + width) : (x - 2.0f);
        float feelerR = feelerL + 2.0f;
        float feelerT = y - 2.0f; // Ngay dưới chân (y hướng lên nên trừ đi)
        float feelerB = y;

        for (GameObject* obj : *coObjects)
        {
            if (obj == this || obj->IsDeleted()) continue;
            Block* block = dynamic_cast<Block*>(obj);
            if (block)
            {
                float sl, st, sr, sb;
                block->GetBoundingBox(sl, st, sr, sb);
                
                // Kiểm tra xem feeler có chạm vào block nào không
                if (feelerL < sr && feelerR > sl && feelerT < sb && feelerB > st)
                {
                    hasGroundAhead = true;
                    break;
                }
            }
        }

        if (!hasGroundAhead)
        {
            vx = -vx; // Đảo chiều
            nx = -nx;
        }
    }
}

void Enemy::Render()
{
    if (died) return;
    Animations::GetInstance()->Get(animationId)->Render(x, y);
}

void Enemy::OnCollision(GameObject* obj)
{
}

void Enemy::SetDied(bool died)
{
    if (died && !this->died)
    {
        this->died = true;
        OnStomped(NULL);
    }
    else
    {
        this->died = died;
    }
}