#include "Pipe.h"
#include "../animation/Animations.h"

Pipe::Pipe(float x, float y, int animationId, bool canEnter, float destX, float destY)
    : GameObject(x, y)
{
    this->animationId = animationId;
    Animation* anim = Animations::GetInstance()->Get(animationId);
    if (anim != NULL) {
        this->width = anim->GetWidth();
        this->height = anim->GetHeight();
    } else {
        this->width = 32;
        this->height = 32;
    }
    this->canEnter = canEnter;
    this->destX = destX;
    this->destY = destY;
    this->isStatic = true;
}

void Pipe::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void Pipe::Render()
{
    Animation* ani = Animations::GetInstance()->Get(animationId);
    if (ani != NULL) ani->Render(x, y);
}