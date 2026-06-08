#pragma once
#include "../core/GameObject.h"

class LuckyBlock : public GameObject
{
private:
    float width;
    float height;
    int activeAnimationId;
    int usedAnimationId;
    bool isHit;

public:
    LuckyBlock(float x, float y, int activeAnimationId, int usedAnimationId);

    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
    void Render() override;

    void Hit();
    bool IsHit() { return isHit; }
};