#include "game/projectiles/Hammer.h"
#include "game/entities/Mario.h"
#include "game/scenes/Map.h"
#include "engine/core/Camera.h"
#include "engine/core/Collision.h"
#include "engine/graphics/Animations.h"
#include "game/entities/Enemy.h"
#include "game/objects/Block.h"

Hammer::Hammer(float x, float y, int direction) : Projectile(x, y, direction) {
    float calc_vx = HAMMER_SPEED_X;
    Mario* mario = Map::GetInstance()->GetMario();
    if (mario != NULL) {
        float distanceX = abs(mario->GetX() - x);
        float flightTime = (2.0f * HAMMER_JUMP_SPEED_Y) / HAMMER_GRAVITY;
        if (flightTime > 0) {
            calc_vx = distanceX / flightTime;
            // Giới hạn lực ném ngang (min 0.04, max 0.15) để búa không ném ra sau lưng hoặc quá nhanh
            if (calc_vx < 0.04f) calc_vx = 0.04f;
            if (calc_vx > 0.15f) calc_vx = 0.15f;
        }
    }
    
    this->vx = direction * calc_vx;
    this->vy = HAMMER_JUMP_SPEED_Y; // Ném vút lên trên (hệ trục Y lên)
    this->width = HAMMER_BBOX_WIDTH;
    this->height = HAMMER_BBOX_HEIGHT;
    this->animationId = HAMMER_ANI;
}

void Hammer::GetBoundingBox(float& left, float& top, float& right, float& bottom) {
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void Hammer::Update(DWORD dt, vector<GameObject*>* coObjects) {
    if (IsDeleted()) return;

    if (isParried) {
        vy = 0;
    } else {
        vy -= HAMMER_GRAVITY * dt; // Rơi xuống (Y giảm)
    }
    
    x += vx * dt;
    y += vy * dt;

    // Tự hủy nếu rơi ra khỏi camera
    Camera* camera = Camera::GetInstance();
    if (camera) {
        if (!camera->IsVisible(x - 32.0f, y - 32.0f, width + 64.0f, height + 64.0f)) {
            Delete();
            return;
        }
    }

    float hl, ht, hr, hb;
    GetBoundingBox(hl, ht, hr, hb);

    if (isParried) {
        for (UINT i = 0; i < coObjects->size(); i++) {
            GameObject* e = coObjects->at(i);
            if (e == this || e->IsDeleted()) continue;

            float sl, st, sr, sb;
            e->GetBoundingBox(sl, st, sr, sb);

            // AABB check
            if (!(hr < sl || hl > sr || hb < st || ht > sb)) {
                if (Enemy* enemy = dynamic_cast<Enemy*>(e)) {
                    if (!enemy->IsDied()) {
                        enemy->OnStomped(NULL);
                        this->Delete();
                        return;
                    }
                }
                else if (Block* block = dynamic_cast<Block*>(e)) {
                    if (!block->IsOneWay()) {
                        this->Delete();
                        return;
                    }
                }
            }
        }
    }
    else {
        // Xuyên địa hình, chỉ check AABB với Mario
        Mario* mario = Map::GetInstance()->GetMario();
        if (mario && !mario->IsDeleted() && !mario->untouchable) {
            float ml, mt, mr, mb;
            mario->GetBoundingBox(ml, mt, mr, mb);
            
            if (hr > ml && hl < mr && hb > mt && ht < mb) {
                // Trúng Mario
                mario->TakeDamage();
            }
        }
    }
}

void Hammer::Render() {
    if (IsDeleted()) return;

    Animation* ani = Animations::GetInstance()->Get(animationId);
    if (ani) {
        // Vẽ hammer, nó sẽ tự lặp qua 4 frame xoay tròn
        ani->Render(x, y);
    }
}
