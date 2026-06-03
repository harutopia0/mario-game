#include "Brick.h"
#include "../animation/animations.h"

Brick::Brick(float x, float y, float width, float height, int animationId) : GameObject(x, y)
{
    this->width = width;
    this->height = height;
    this->animationId = animationId;
    this->isStatic = true;
}

void Brick::Update(DWORD dt)
{
}

void Brick::Render()
{
    Animation* ani = Animations::GetInstance()->Get(this->animationId);
    if (ani != NULL) ani->Render(x, y);
}

void Brick::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + this->width;
    bottom = y + this->height;
}