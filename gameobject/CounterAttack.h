#pragma once
#include "../core/GameObject.h"
#include <vector>

class Mario;

class CounterAttack : public GameObject {
private:
    Mario* mario;
    ULONGLONG startTime;
    float width;
    float height;

public:
    CounterAttack(Mario* mario);
    void Update(DWORD dt, std::vector<GameObject*>* coObjects) override;
    void Render() override;
    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
};
