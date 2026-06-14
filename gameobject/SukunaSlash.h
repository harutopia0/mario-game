#pragma once
#include "../core/GameObject.h"
#include "Enemy.h"

class SukunaSlash : public GameObject {
private:
    Enemy* targetEnemy;
    ULONGLONG startTime;
    bool hasDamaged;

public:
    SukunaSlash(float x, float y, Enemy* enemy);
    void Update(DWORD dt, vector<GameObject*>* coObjects) override;
    void Render() override;
    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
};
