#include "Buff.h"
#include "../animation/Animations.h"

Buff::Buff(float x, float y, int animationId)
    : GameObject(x, y)
{
    this->animationId = animationId;
    Animation* anim = Animations::GetInstance()->Get(animationId);
    if (anim != NULL) {
        this->width = anim->GetWidth();
        this->height = anim->GetHeight();
    } else {
        this->width = BUFF_WIDTH;
        this->height = BUFF_HEIGHT;
    }

    vx = 0.0f;
    vy = 0.0f;

    isStatic = true;
	isDeleted = false;
}

void Buff::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void Buff::Update(DWORD dt, vector<GameObject*>* coObjects)
{
}

void Buff::Render()
{
    Animation* ani = Animations::GetInstance()->Get(animationId);

    if (ani != NULL)
    {
        ani->Render(x, y);
    }
}