#include "Brick.h"
#include "../animation/animations.h"

Brick::Brick(float x, float y) : GameObject(x, y) {}

void Brick::Update(DWORD dt)
{

}

void Brick::Render()
{
    Animation* ani = Animations::GetInstance()->Get(201);
    if (ani != NULL) ani->Render(x, y);
}

void Brick::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + 48.0f;
    bottom = y + 48.0f;
}