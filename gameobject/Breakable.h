#pragma once
#include "DynamicBlock.h"

class Breakable : public DynamicBlock
{
private:
    float width;
    float height;
    int animationId;

public:
    Breakable(float x, float y, int animationId);

    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
    void Render() override;

    virtual void Break(bool dropItem = true);
};