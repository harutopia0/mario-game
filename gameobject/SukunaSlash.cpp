#include "SukunaSlash.h"
#include "../render/Sprites.h"

SukunaSlash::SukunaSlash(float x, float y, Enemy* enemy) : GameObject(x, y) {
    targetEnemy = enemy;
    startTime = GetTickCount64();
    hasDamaged = false;
    layer = LAYER_PLAYER; // Vẽ đè lên trên nhân vật và quái
}

void SukunaSlash::Update(DWORD dt, vector<GameObject*>* coObjects) {
    if (isDeleted) return;

    ULONGLONG elapsed = GetTickCount64() - startTime;

    // Khoảng frame 9 (từ 400ms trở đi), quái vật mới chính thức chết (khi nổ chém xảy ra)
    if (elapsed >= 400 && !hasDamaged) {
        if (targetEnemy && !targetEnemy->IsDied()) {
            targetEnemy->SetDied(true);
        }
        hasDamaged = true;
    }

    // Sau khi kết thúc hoạt ảnh 800ms (16 frames * 50ms)
    if (elapsed >= 800) {
        this->Delete();
    }
}

void SukunaSlash::Render() {
    ULONGLONG elapsed = GetTickCount64() - startTime;
    int frameIndex = (int)(elapsed / 50);
    if (frameIndex > 15) frameIndex = 15;
    
    // Đọc trực tiếp Sprite từ Sprites instance theo ID tăng dần
    int spriteId = 801 + frameIndex;
    Sprite* sprite = Sprites::GetInstance()->Get(spriteId);
    if (sprite != NULL) {
        sprite->Draw(x, y);
    }
}

void SukunaSlash::GetBoundingBox(float& left, float& top, float& right, float& bottom) {
    // Hiệu ứng không có va chạm vật lý với Mario hay các gạch đá khác
    left = 0;
    top = 0;
    right = 0;
    bottom = 0;
}
