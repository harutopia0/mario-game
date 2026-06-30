#pragma once
#include "Enemy.h"

#define HAMMERBRO_SPEED_X 0.02f
#define HAMMERBRO_JUMP_SPEED 0.3375f
#define HAMMERBRO_GRAVITY 0.0008f

#define HAMMERBRO_BBOX_WIDTH 16.0f
#define HAMMERBRO_BBOX_HEIGHT 24.0f

// States
#define HAMMERBRO_STATE_WALKING 100
#define HAMMERBRO_STATE_JUMPING 200
#define HAMMERBRO_STATE_DIE 300

// Animations
#define HAMMERBRO_ANI_WALK_LEFT 11001
#define HAMMERBRO_ANI_WALK_RIGHT 11002
#define HAMMERBRO_ANI_THROW_LEFT 11003
#define HAMMERBRO_ANI_THROW_RIGHT 11004

class HammerBro : public Enemy {
private:
    float startX;
    float startY;
    
    int state;
    bool isThrowing;
    ULONGLONG throwStart;
    ULONGLONG throwCooldownStart;
    
    ULONGLONG jumpCooldownStart;
    bool isOnGround;
    bool isDroppingDown;

    float currentWalkingDirection; // Để lưu hướng di chuyển thực tế độc lập với hướng quay mặt

public:
    HammerBro(float x, float y);
    virtual void Update(DWORD dt, vector<GameObject*>* coObjects) override;
    virtual void Render() override;
    virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
    virtual void OnStomped(Mario* mario) override;
};
