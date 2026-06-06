#pragma once
#include "../core/GameObject.h"

class Brick : public GameObject
{
private:
    float width;
    float height;
    int animationId;

public:
    Brick(float x, float y, int animationId);

    virtual void Update(DWORD dt);
    virtual void Render();
    virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom);
};