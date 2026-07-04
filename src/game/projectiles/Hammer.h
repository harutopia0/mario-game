#pragma once
#include "game/projectiles/Projectile.h"

#define HAMMER_SPEED_X 0.08f
#define HAMMER_JUMP_SPEED_Y 0.25f
#define HAMMER_GRAVITY 0.0007f

#define HAMMER_BBOX_WIDTH 15.0f
#define HAMMER_BBOX_HEIGHT 15.0f

#define HAMMER_ANI 11000

class Hammer : public Projectile
{
  private:
    float vy;

  public:
    Hammer(float x, float y, int direction);
    virtual void Update(DWORD dt, vector<GameObject *> *coObjects) override;
    virtual void Render() override;
    virtual void GetBoundingBox(float &left, float &top, float &right, float &bottom) override;
};
