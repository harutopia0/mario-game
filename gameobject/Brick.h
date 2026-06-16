#pragma once
#include "DynamicBlock.h"

class Brick : public DynamicBlock
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
    virtual void Break(bool dropItem = true);
};