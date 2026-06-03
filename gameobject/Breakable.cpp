#include "Breakable.h"
#include "../animation/Animations.h"

Breakable::Breakable(float x, float y, float width, float height, int animationId)
    : GameObject(x, y)
{
    this->width = width;
    this->height = height;
    this->animationId = animationId;
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