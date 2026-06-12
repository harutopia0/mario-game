#pragma once
#include "../core/GameObject.h"

class Breakable : public GameObject
{
private:
    float width;
    float height;
    int animationId;

public:
    Breakable(float x, float y, int animationId);

    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
    void Render() override;

    void Break();
};