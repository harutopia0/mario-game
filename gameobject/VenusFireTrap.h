#pragma once
#include "Enemy.h"
#include "EnemyFireball.h"
#include "Mario.h"

#define VENUS_STATE_HIDING 100
#define VENUS_STATE_GOING_UP 200
#define VENUS_STATE_AIMING 300
#define VENUS_STATE_SHOOTING 400
#define VENUS_STATE_GOING_DOWN 500
#define VENUS_STATE_DIE        600

#define VENUS_DIR_UP_LEFT 1
#define VENUS_DIR_DOWN_LEFT 2
#define VENUS_DIR_UP_RIGHT 3
#define VENUS_DIR_DOWN_RIGHT 4

#define VENUS_SPEED_Y 0.03f
#define VENUS_SAFE_ZONE_WIDTH 32.0f
#define VENUS_SHOOT_DELAY 750 // Thời gian ngắm trước khi bắn
#define VENUS_HIDE_DELAY 2000 // Thời gian nằm dưới ống

class VenusFireTrap : public Enemy {
    bool isUpsideDown;
    int aimDir;
    int state;
    float hiddenY;
    float poppedY;
    DWORD timerStart;
    
    // Lưu các animation ID để tiện sử dụng tuỳ theo hướng
    int aniUpLeft, aniDownLeft, aniUpRight, aniDownRight;

public:
    VenusFireTrap(float x, float y, bool isUpsideDown);
    void Update(DWORD dt, vector<GameObject*>* coObjects) override;
    void Render() override;
    void GetBoundingBox(float &left, float &top, float &right, float &bottom) override;
    
    void SetState(int state);
    void DetermineAimDirection(Mario* mario);
    void ShootFireball(vector<GameObject*>* coObjects);
    void OnStomped(Mario* mario) override;
};
