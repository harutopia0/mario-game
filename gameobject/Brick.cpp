#include "Brick.h"
#include "../animation/animations.h"

Brick::Brick(float x, float y, int animationId) : GameObject(x, y)
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