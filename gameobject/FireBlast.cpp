#include "FireBlast.h"
#include "../core/Game.h"
#include "../gameobject/DynamicBlock.h"
#include "../gameobject/Enemy.h"
#include "../physics/Collision.h"


FireBlast::FireBlast(float x, float y, int direction)
    : Projectile(x, y, direction) {
  this->width = FIREBLAST_WIDTH;
  this->height = FIREBLAST_HEIGHT;
  this->vx = FIREBLAST_SPEED * direction;
  this->vy = 0;
  if (direction > 0)
    this->animationId = 610;
  else
    this->animationId = 611;
  this->startX = x;
}

void FireBlast::Update(DWORD dt, vector<GameObject *> *coObjects) {
  x += vx * dt;
  y += vy * dt;

  if (x - startX > 400 || startX - x > 400) {
    Delete();
    return;
  }

  float l, t, r, b;
  GetBoundingBox(l, t, r, b);

  if (coObjects != NULL) {
    for (size_t i = 0; i < coObjects->size(); i++) {
      GameObject *obj = coObjects->at(i);
      if (obj->IsDeleted())
        continue;

      float ol, ot, oright, ob;
      obj->GetBoundingBox(ol, ot, oright, ob);

      if (r > ol && l < oright && b > ot && t < ob) {
        // Va chạm AABB
        if (Enemy *enemy = dynamic_cast<Enemy *>(obj)) {
          enemy->SetDied(true);
        } else if (DynamicBlock *block = dynamic_cast<DynamicBlock *>(obj)) {
          block->Break();
        }
      }
    }
  }
}
