#pragma once
#include "game/projectiles/Projectile.h"

#define FIREBALL_WIDTH 8.0f
#define FIREBALL_HEIGHT 8.0f
#define FIREBALL_SPEED_X 0.15f
#define FIREBALL_BOUNCE_SPEED 0.2f
#define FIREBALL_GRAVITY -0.00067f

#define FIREBALL_STATE_NORMAL 1
#define FIREBALL_STATE_EXPLODING 2

class Fireball : public Projectile {
    int state;
    DWORD explodeStart;
public:
    Fireball(float x, float y, int direction);
    void Update(DWORD dt, vector<GameObject*>* coObjects) override;
    void Render() override;
};
