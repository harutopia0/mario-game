#include "Projectile.h"
#include "../animation/Animations.h"

Projectile::Projectile(float x, float y, int direction) : GameObject(x, y) {
    this->nx = direction;
    this->layer = LAYER_ITEMS;
}

void Projectile::GetBoundingBox(float& left, float& top, float& right, float& bottom) {
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void Projectile::Render() {
    Animation* ani = Animations::GetInstance()->Get(animationId);
    if (ani != NULL) {
        ani->Render(x, y);
    }
}
