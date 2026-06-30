#pragma once
#include "StaticBlock.h"

class Platform : public StaticBlock
{
private:
    float width;
    float height;
    int animationId;

public:
    Platform(float x, float y, int animationId);

    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
    void Render() override;
};