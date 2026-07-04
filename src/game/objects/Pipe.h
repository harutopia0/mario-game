#pragma once
#include "game/objects/StaticBlock.h"

class Pipe : public StaticBlock
{
private:
    int pipeHeight; // in blocks (16px each)
    bool canEnter;
    float destX, destY;
    float width, height;

public:
    Pipe(float x, float y, int pipeHeight, bool canEnter, float destX, float destY);

    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
    void Render() override;

    bool CanEnter() const { return canEnter; }
    float GetDestX() const { return destX; }
    float GetDestY() const { return destY; }
    float GetWidth() const { return width; }
};