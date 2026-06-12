#pragma once
#include "../core/GameObject.h"

class Pipe : public GameObject
{
private:
    float width;
    float height;
    int animationId;
    bool canEnter;
    float destX, destY;

public:
    Pipe(float x, float y, int animationId, bool canEnter = false, float destX = 0, float destY = 0);

    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
    void Render() override;

    bool CanEnter() const { return canEnter; }
    float GetDestX() const { return destX; }
    float GetDestY() const { return destY; }
    float GetWidth() const { return width; }
};