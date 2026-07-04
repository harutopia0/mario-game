#pragma once
#include "game/entities/Enemy.h"

#define PIRANHA_STATE_HIDING 100
#define PIRANHA_STATE_GOING_UP 200
#define PIRANHA_STATE_BITING 300
#define PIRANHA_STATE_GOING_DOWN 400
#define PIRANHA_STATE_DIE 500

#define PIRANHA_BITE_TIME 1500
#define PIRANHA_HIDE_TIME 1500

#define PIRANHA_SPEED 0.03f
#define PIRANHA_SAFE_ZONE_WIDTH 35.0f // Mario đứng gần sẽ không chui lên

class PiranhaPlant : public Enemy
{
  private:
    int state;
    ULONGLONG waitTimeStart;
    float hiddenY;
    float poppedY;

  public:
    PiranhaPlant(float x, float y);
    virtual void Update(DWORD dt, vector<GameObject *> *coObjects) override;
    virtual void Render() override;
    virtual void GetBoundingBox(float &left, float &top, float &right, float &bottom) override;
    virtual void OnCollision(GameObject *obj) override;
    virtual void OnStomped(Mario *mario) override;
};
