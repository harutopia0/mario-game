#pragma once
#include "DynamicBlock.h"

class LuckyBlock : public DynamicBlock
{
private:
    float width;
    float height;
    int activeAnimationId;
    int usedAnimationId;
    bool isHit;
    void SpawnItem();

public:
    LuckyBlock(float x, float y, int activeAnimationId, int usedAnimationId);

    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
    void Render() override;

    void Hit();
    bool IsHit() { return isHit; }

    bool IsOneWay() const override { return isHit; }

    virtual void Break(bool dropItem = true);
};