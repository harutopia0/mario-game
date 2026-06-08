#include "Breakable.h"
#include "../animation/Animations.h"

Breakable::Breakable(float x, float y, int animationId) : GameObject(x, y)
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
    this->isStatic = true;
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

void Breakable::Break()
{
    this->Delete();
}