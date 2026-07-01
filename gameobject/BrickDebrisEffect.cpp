#include "BrickDebrisEffect.h"
#include "../animation/Animations.h"

BrickDebrisEffect::BrickDebrisEffect(float x, float y, int animationId) : GameObject(x, y) {
    this->animationId = animationId;
    this->layer = LAYER_EFFECTS;
    
    // Khởi tạo 4 mảnh vụn
    // Mảnh 1: Trái trên
    pieces[0].x = x; pieces[0].y = y + 8.0f;
    pieces[0].vx = -0.06f; pieces[0].vy = 0.3f;
    
    // Mảnh 2: Phải trên
    pieces[1].x = x + 8.0f; pieces[1].y = y + 8.0f;
    pieces[1].vx = 0.06f; pieces[1].vy = 0.3f;
    
    // Mảnh 3: Trái dưới
    pieces[2].x = x; pieces[2].y = y;
    pieces[2].vx = -0.08f; pieces[2].vy = 0.15f;
    
    // Mảnh 4: Phải dưới
    pieces[3].x = x + 8.0f; pieces[3].y = y;
    pieces[3].vx = 0.08f; pieces[3].vy = 0.15f;
}

void BrickDebrisEffect::Update(DWORD dt, vector<GameObject*>* coObjects) {
    if (isDeleted) return;

    bool allOut = true;

    for (int i = 0; i < 4; i++) {
        pieces[i].x += pieces[i].vx * dt;
        pieces[i].y += pieces[i].vy * dt;
        pieces[i].vy -= 0.001f * dt; // Trọng lực kéo xuống

        if (pieces[i].y > -50.0f) { // Chắc chắn mảnh chưa rơi quá sâu xuống dưới màn hình (Y=0 là đáy map)
            allOut = false;
        }
    }
    
    if (allOut) {
        this->isDeleted = true;
    }
}

void BrickDebrisEffect::Render() {
    Animation* anim = Animations::GetInstance()->Get(animationId);
    if (!anim) return;
    
    for (int i = 0; i < 4; i++) {
        anim->Render(pieces[i].x, pieces[i].y, 8.0f, 8.0f);
    }
}

void BrickDebrisEffect::GetBoundingBox(float& left, float& top, float& right, float& bottom) {
    // Không có va chạm
    left = 0; top = 0; right = 0; bottom = 0;
}
