#pragma once
#include "../core/GameObject.h"

class Brick : public GameObject
{
public:
    Brick(float x, float y);
    virtual void Update(DWORD dt);
    virtual void Render();
    virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom);
};