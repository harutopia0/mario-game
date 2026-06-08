#include "../core/GameObject.h"
#include "../render/Sprites.h"

GameObject::GameObject(float x, float y)
{
    this->x = x;
    this->y = y;
    this->vx = 0.0f;
    this->vy = 0.0f;
    this->nx = 1;

    gridRow = -1;
    gridCol = -1;
    isDeleted = false;
}

void GameObject::Update(DWORD dt, vector<GameObject*>* coObjects)
{
    x += vx * dt;
    y += vy * dt;
}

void GameObject::RenderBoundingBox()
{
    float l, t, r, b;
    GetBoundingBox(l, t, r, b);

    float width = r - l;
    float height = b - t;

    Sprite* bbox = Sprites::GetInstance()->Get(99999);
    if (bbox != NULL)
    {
        bbox->Draw(l, t, width, height, 0.4f);
    }
}
void GameObject::Delete()
{
    isDeleted = true;
}
bool GameObject::IsDeleted() const
{
    return isDeleted;
}