#pragma once
#include "Projectile.h"

class SukunaProjectile : public Projectile {
private:
    ULONGLONG creationTime;

public:
    SukunaProjectile(float x, float y, int direction);
    void Update(DWORD dt, vector<GameObject*>* coObjects) override;
    void Render() override;
};
