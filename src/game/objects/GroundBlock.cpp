#include "game/objects/GroundBlock.h"
#include "engine/graphics/Animations.h"

GroundBlock::GroundBlock(float x, float y, int animationId, float customWidth)
    : StaticBlock(x, y)
{
    this->animationId = animationId;
    this->width = customWidth > 0 ? customWidth : 16.0f;
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
