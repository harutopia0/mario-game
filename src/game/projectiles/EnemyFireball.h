#pragma once
#include "game/projectiles/Projectile.h"

#define ENEMY_FIREBALL_WIDTH 8.0f
#define ENEMY_FIREBALL_HEIGHT 8.0f
#define ENEMY_FIREBALL_SPEED 0.05f

class EnemyFireball : public Projectile
{
  public:
    EnemyFireball(float x, float y, float vx, float vy);
    void Update(DWORD dt, vector<GameObject *> *coObjects) override;
    void Render() override;
};
