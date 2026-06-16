#include "Breakable.h"
#include "../animation/Animations.h"

Breakable::Breakable(float x, float y, int animationId) : DynamicBlock(x, y)
{
    this->animationId = animationId;
    Animation* anim = Animations::GetInstance()->Get(animationId);
    if (anim != NULL) {
        this->width = anim->GetWidth();
        this->height = anim->GetHeight();
    } else {
        this->width = 16;
        this->height = 16;
    }
    // this->isStatic is already set by DynamicBlock
}

void Breakable::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void Breakable::Render()
{
    Animation* ani = Animations::GetInstance()->Get(animationId);
    if (ani != NULL) ani->Render(x, y);
}

void Breakable::Break(bool dropItem)
{
    this->Delete();
}