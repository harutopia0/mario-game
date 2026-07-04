#include "game/items/Prop.h"
#include "engine/graphics/Sprites.h"

Prop::Prop(float x, float y, int spriteId, float width, float height) : GameObject(x, y)
{
    this->spriteId = spriteId;
    this->width = width;
    this->height = height;
    this->layer = LAYER_PROP; // Trở lại làm cảnh nền đằng sau
}

void Prop::Render()
{
    Sprite *sprite = Sprites::GetInstance()->Get(spriteId);
    if (sprite)
    {
        sprite->Draw(x, y);
    }
}

void Prop::GetBoundingBox(float &l, float &t, float &r, float &b)
{
    l = x;
    t = y;
    r = x + width;
    b = y + height;
}
