#include "LuckyBlock.h"
#include "../animation/Animations.h"

LuckyBlock::LuckyBlock(float x, float y, int activeAnimationId, int usedAnimationId)
    : GameObject(x, y)
{
    this->activeAnimationId = activeAnimationId;
    this->usedAnimationId = usedAnimationId;
    Animation* anim = Animations::GetInstance()->Get(activeAnimationId);
    if (anim != NULL) {
        this->width = anim->GetWidth();
        this->height = anim->GetHeight();
    } else {
        this->width = 16;
        this->height = 16;
    }
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