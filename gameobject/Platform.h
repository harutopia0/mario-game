#pragma once
#include "../core/GameObject.h"

class Platform : public GameObject
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