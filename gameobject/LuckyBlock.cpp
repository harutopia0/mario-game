#include "LuckyBlock.h"
#include "../animation/Animations.h"

LuckyBlock::LuckyBlock(float x, float y, int activeAnimationId, int usedAnimationId)
    : DynamicBlock(x, y)
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
    // this->isStatic is already set by DynamicBlock
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

void LuckyBlock::Break(bool dropItem)
{
    if (!isHit && dropItem) {
        // TODO: Spawn item (Mushroom/Flower/Coin) here
        OutputDebugStringA("LuckyBlock broken! Dropping item...\n");
    }
    this->Delete();
}