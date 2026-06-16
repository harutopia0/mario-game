#include "GroundBlock.h"
#include "../animation/Animations.h"

GroundBlock::GroundBlock(float x, float y, int animationId)
    : StaticBlock(x, y)
{
    this->animationId = animationId;
    this->width = 16.0f;
    this->height = 16.0f;
}

void GroundBlock::Render()
{
    Animation* ani = Animations::GetInstance()->Get(animationId);
    if (ani != NULL) {
        ani->Render(x, y);
    }
}

void GroundBlock::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}
