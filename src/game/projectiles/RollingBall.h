#pragma once
#include "game/projectiles/Projectile.h"

#define ROLLINGBALL_SPEED_X 0.2f
#define ROLLINGBALL_GRAVITY -0.002f
#define ROLLINGBALL_WIDTH 34.0f
#define ROLLINGBALL_HEIGHT 34.0f

#define ROLLINGBALL_STATE_NORMAL 0
#define ROLLINGBALL_STATE_EXPLODING 1

class RollingBall : public Projectile
{
  protected:
    int state;
    ULONGLONG explodeStart;
    float rotationAngle;

  public:
    RollingBall(float x, float y, int direction);
    virtual void Update(DWORD dt, vector<GameObject *> *coObjects) override;
    virtual void Render() override;

    void Explode();
};
