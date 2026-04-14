#include "../gameplay/GameObject.h"

GameObject::GameObject(float x, float y)
{
    this->x = x;
    this->y = y;
    this->vx = 0.0f;
    this->vy = 0.0f;
    this->nx = 1;
}

void GameObject::Update(DWORD dt)
{
    x += vx * dt;
    y += vy * dt;
}
