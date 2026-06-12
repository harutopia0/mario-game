#pragma once
#include "../core/GameObject.h"

class Projectile : public GameObject {
protected:
    float width;
    float height;
    int animationId;

public:
    Projectile(float x, float y, int direction);
    virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom);
    virtual void Update(DWORD dt, vector<GameObject*>* coObjects) = 0;
    virtual void Render();
};
