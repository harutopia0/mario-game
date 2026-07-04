#include "game/objects/Platform.h"
#include "engine/graphics/Animations.h"

Platform::Platform(float x, float y, int animationId) : StaticBlock(x, y)
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
}

void Platform::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + this->width;
    bottom = y + this->height;
}

void Platform::Render()
{
    Animation* ani = Animations::GetInstance()->Get(this->animationId);
    if (ani != NULL) ani->Render(x, y);
}