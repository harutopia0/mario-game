#pragma once
#include "engine/core/GameObject.h"

#define ENEMY_GRAVITY -0.002f

class Mario;

class Enemy : public GameObject
{
  protected:
    float width, height;
    int animationId;
    bool died;
    bool isFreezed;
    int nx;

    virtual bool CanFallOffLedge()
    {
        return false;
    }

  public:
    Enemy(float x, float y, int animationId);
    virtual void GetBoundingBox(float &left, float &top, float &right, float &bottom) override;
    virtual void Update(DWORD dt, vector<GameObject *> *coObjects) override;
    virtual void Render() override;
    virtual void OnCollision(GameObject *obj);
    virtual void OnStomped(Mario *mario)
    {
    }

    bool IsDied() const
    {
        return died;
    }
    void SetDied(bool died);
    bool IsFreezed() const
    {
        return isFreezed;
    }
    void SetFreezed(bool frozen)
    {
        this->isFreezed = frozen;
    }
    virtual ~Enemy()
    {
    }
};
