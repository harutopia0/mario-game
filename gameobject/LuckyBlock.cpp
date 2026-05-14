#include "LuckyBlock.h"
#include "../animation/Animations.h"

LuckyBlock::LuckyBlock(float x, float y, float width, float height, int activeAnimationId, int usedAnimationId)
    : GameObject(x, y)
{
    this->width = width;
    this->height = height;
    this->activeAnimationId = activeAnimationId;
    this->usedAnimationId = usedAnimationId;
    this->isHit = false;
    this->isStatic = true;
}

void LuckyBlock::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void LuckyBlock::Render()
{
    int aniId = isHit ? usedAnimationId : activeAnimationId;
    Animation* ani = Animations::GetInstance()->Get(aniId);
    if (ani != NULL) ani->Render(x, y);
}

void LuckyBlock::Hit()
{
    if (!isHit) {
        isHit = true;
    }
}