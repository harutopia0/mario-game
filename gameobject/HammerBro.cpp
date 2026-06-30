#include "HammerBro.h"
#include "Hammer.h"
#include "Mario.h"
#include "../audio/AudioManager.h"
#include "../gameplay/Map.h"
#include "../render/Camera.h"
#include "../animation/Animations.h"
#include "../physics/Collision.h"
#include "Block.h"
#include <cstdlib> // rand()

HammerBro::HammerBro(float x, float y) : Enemy(x, y, 5002) {
    this->startX = x;
    this->startY = y;
    this->width = HAMMERBRO_BBOX_WIDTH;
    this->height = HAMMERBRO_BBOX_HEIGHT;

    this->state = HAMMERBRO_STATE_WALKING;
    this->vx = -HAMMERBRO_SPEED_X; // Mặc định đi sang trái
    this->currentWalkingDirection = -1;
    this->nx = -1; // Quay mặt sang trái

    this->isThrowing = false;
    this->throwStart = 0;
    this->throwCooldownStart = GetTickCount64();
    this->jumpCooldownStart = GetTickCount64();
    this->isOnGround = false;
    this->isDroppingDown = false;
}

void HammerBro::GetBoundingBox(float& left, float& top, float& right, float& bottom) {
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void HammerBro::Update(DWORD dt, vector<GameObject*>* coObjects) {
    if (isDeleted) return;
    
    if (state == HAMMERBRO_STATE_DIE) {
        vy -= HAMMERBRO_GRAVITY * dt;
        x += vx * dt;
        y += vy * dt;
        if (y < -100.0f) {
            Delete();
        }
        return;
    }
    
    if (state == HAMMERBRO_STATE_FLAT) {
        vx = 0.0f;
        vy = 0.0f;
        if (GetTickCount64() - flatTimeStart > 500) {
            Delete();
        }
        return;
    }
    
    Camera* camera = Camera::GetInstance();
    if (camera) {
        if (!camera->IsVisible(x - 16.0f, y - 16.0f, width + 32.0f, height + 32.0f)) {
            // Khi ở ngoài camera, reset lại các timer để khi lọt vào không bị tấn công / nhảy ngay lập tức
            throwCooldownStart = GetTickCount64();
            jumpCooldownStart = GetTickCount64();
            return;
        }
    }

    // Cập nhật hướng mặt về phía Mario
    Mario* mario = Map::GetInstance()->GetMario();
    if (mario) {
        if (mario->GetX() > this->x) nx = 1;
        else nx = -1;
    }

    // Logic di chuyển
    if (state == HAMMERBRO_STATE_WALKING) {
        if (x < startX - 32.0f) {
            currentWalkingDirection = 1;
            vx = HAMMERBRO_SPEED_X;
        } else if (x > startX + 32.0f) {
            currentWalkingDirection = -1;
            vx = -HAMMERBRO_SPEED_X;
        } else {
            vx = currentWalkingDirection * HAMMERBRO_SPEED_X;
        }
    } else {
        vx = 0.0f; // Đứng lại khi chuẩn bị ném hoặc đang nhảy
    }

    // Trọng lực (hướng xuống, làm giảm vy)
    vy -= HAMMERBRO_GRAVITY * dt;

    float dx = vx * dt;
    float dy = vy * dt;

    float ml, mt, mr, mb;
    GetBoundingBox(ml, mt, mr, mb);

    float min_tx = 1.0f;
    float nx_col = 0;

    // Va chạm trục X
    for (GameObject* obj : *coObjects) {
        if (obj == this || obj->IsDeleted()) continue;
        Block* block = dynamic_cast<Block*>(obj);
        if (block) {
            float sl, st, sr, sb;
            block->GetBoundingBox(sl, st, sr, sb);
            if (mb > st && mt < sb) {
                float t, temp_nx, temp_ny;
                Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, dx, 0.0f, sl, st, sr, sb, t, temp_nx, temp_ny);
                if (t < min_tx && temp_nx != 0) {
                    if (block->IsOneWay()) continue; // Bỏ qua va chạm ngang với nền OneWay
                    min_tx = t;
                    nx_col = temp_nx;
                }
            }
        }
    }
    x += min_tx * dx + nx_col * 0.01f;
    if (nx_col != 0) {
        currentWalkingDirection = -currentWalkingDirection; // Quay đầu khi đụng tường
        vx = currentWalkingDirection * HAMMERBRO_SPEED_X;
    }

    GetBoundingBox(ml, mt, mr, mb);

    float min_ty = 1.0f;
    float ny_col = 0;

    // Va chạm trục Y
    for (GameObject* obj : *coObjects) {
        if (obj == this || obj->IsDeleted()) continue;
        Block* block = dynamic_cast<Block*>(obj);
        if (block) {
            float sl, st, sr, sb;
            block->GetBoundingBox(sl, st, sr, sb);
            
            // HammerBro nhảy xuyên nền từ dưới lên. Chỉ xét va chạm khi rớt xuống (dy < 0)
            if (dy < 0 && mr > sl && ml < sr) {
                float t, temp_nx, temp_ny;
                Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, 0.0f, dy, sl, st, sr, sb, t, temp_nx, temp_ny);
                if (t < min_ty && temp_ny == 1) { // 1 là va chạm vào mặt trên của block (khi rớt xuống)
                    min_ty = t;
                    ny_col = temp_ny;
                }
            }
        }
    }

    y += min_ty * dy + ny_col * 0.01f;
    
    if (ny_col == 1) {
        vy = 0;
        isOnGround = true;
    } else {
        isOnGround = false;
    }

    // Xử lý ném búa
    bool canThrow = true;
    if (mario == NULL || mario->IsDied() || mario->IsDeleted()) {
        canThrow = false;
    }

    if (!isThrowing) {
        if (canThrow && GetTickCount64() - throwCooldownStart > 2500) { // Mỗi 2.5s ném 1 lần
            isThrowing = true;
            throwStart = GetTickCount64();
            state = HAMMERBRO_STATE_WALKING; // Vẫn vừa đi vừa ném được
        }
    } else {
        if (GetTickCount64() - throwStart > 300) { // Khựng 300ms rồi ném
            float spawnX = (nx > 0) ? (x + width) : (x - HAMMER_BBOX_WIDTH);
            float spawnY = y + height - 8.0f;

            Hammer* hammer = new Hammer(spawnX, spawnY, nx);
            Map::GetInstance()->GetObjects().push_back(hammer);
            Map::GetInstance()->AddObjectToGrid(hammer);
            coObjects->push_back(hammer);

            isThrowing = false;
            throwCooldownStart = GetTickCount64();
        }
    }

    // Xử lý nhảy
    if (isOnGround && GetTickCount64() - jumpCooldownStart > 4000) {
        // Nhảy mỗi 4s
        vy = HAMMERBRO_JUMP_SPEED;
        jumpCooldownStart = GetTickCount64();
        // Trong game thật HammerBro có thể drop xuống bằng cách bỏ qua collision block
        // Để đơn giản, chỉ cho nó nhảy lên
    }

    // Gán animation
    if (nx > 0) {
        if (isThrowing) animationId = HAMMERBRO_ANI_THROW_RIGHT;
        else animationId = HAMMERBRO_ANI_WALK_RIGHT;
    } else {
        if (isThrowing) animationId = HAMMERBRO_ANI_THROW_LEFT;
        else animationId = HAMMERBRO_ANI_WALK_LEFT;
    }
}

void HammerBro::Render() {
    if (isDeleted) return;
    
    if (state == HAMMERBRO_STATE_DIE) {
        int aniId = (nx > 0) ? HAMMERBRO_ANI_WALK_RIGHT : HAMMERBRO_ANI_WALK_LEFT;
        Animation* ani = Animations::GetInstance()->Get(aniId);
        if (ani) {
            ani->Render(x, y, 0, 1); // nx = 0, ny = 1 (vertical flip)
        }
        return;
    }

    if (state == HAMMERBRO_STATE_FLAT) {
        int aniId = (nx > 0) ? HAMMERBRO_ANI_WALK_RIGHT : HAMMERBRO_ANI_WALK_LEFT;
        Animation* ani = Animations::GetInstance()->Get(aniId);
        if (ani) {
            float shiftY = -height * 0.4f; // Align feet with ground (y increases upwards, so shift down is negative)
            ani->RenderScaled(x, y + shiftY, 1.0f, 0.2f);
        }
        return;
    }
    
    Animation* ani = Animations::GetInstance()->Get(animationId);
    if (ani) {
        ani->Render(x, y);
    }
}

void HammerBro::OnStomped(Mario* mario) {
    if (state == HAMMERBRO_STATE_DIE || state == HAMMERBRO_STATE_FLAT) return;

    died = true;
    layer = LAYER_BACKGROUND; // No collision anymore

    if (mario != NULL) {
        // Bị dẫm -> Xẹp xuống
        state = HAMMERBRO_STATE_FLAT;
        flatTimeStart = GetTickCount64();
        vx = 0.0f;
        vy = 0.0f;
    } else {
        // Bị đòn khác -> Lật ngược rơi đi
        state = HAMMERBRO_STATE_DIE;
        vy = HAMMERBRO_JUMP_SPEED * 0.5f; // Jump upward slightly
        vx = (nx > 0) ? -0.05f : 0.05f; // Move slightly in the opposite horizontal direction
    }

    AudioManager::GetInstance()->PlaySFX("stomp");
}
