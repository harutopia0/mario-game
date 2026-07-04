#pragma once
#include "engine/core/GameObject.h"

class Projectile : public GameObject
{
  protected:
    float width;
    float height;
    int animationId;
    bool isParried = false;

  public:
    Projectile(float x, float y, int direction);
    virtual void GetBoundingBox(float &left, float &top, float &right, float &bottom);
    virtual void Update(DWORD dt, vector<GameObject *> *coObjects) = 0;
    virtual void Render();

    bool IsParried() const
    {
        return isParried;
    }
    void SetParried(bool parried)
    {
        isParried = parried;
    }
    void Deflect(int marioDir)
    {
        isParried = true;
        this->nx = marioDir;
        float originalSpeed = this->vx < 0.0f ? -this->vx : this->vx;
        float speed = originalSpeed > 0.01f ? originalSpeed : 0.15f;
        this->vx = marioDir * speed;
        this->vy = 0.0f;
    }
};
