#pragma once
#include "Projectile.h"
#include <vector>

#define FIREBLAST_SPEED 0.3f
#define FIREBLAST_WIDTH 71.0f
#define FIREBLAST_HEIGHT 23.0f

class FireBlast : public Projectile {
private:
    float startX;
public:
    FireBlast(float x, float y, int direction);
    void Update(DWORD dt, vector<GameObject*>* coObjects) override;
};
