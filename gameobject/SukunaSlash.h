#pragma once
#include "../core/GameObject.h"
#include "Enemy.h"
#include <vector>

struct SlashInstance {
    int spriteId; // 630 or 631
    float offsetX;
    float offsetY;
    float rotation; // angle in radians
    float shrinkDelay;
    float shrinkDuration;
};

class SukunaSlash : public GameObject {
private:
    float width;
    float height;
    Enemy* targetEnemy;
    ULONGLONG startTime;
    ULONGLONG shrinkStartTime;
    bool hasDamaged;

    bool isSlashPhase;
    bool isShrinkPhase;
    int direction;
    ULONGLONG meleeCheckStartTime;

    std::vector<SlashInstance> slashes;

public:
    SukunaSlash(float x, float y, int direction);
    void Update(DWORD dt, std::vector<GameObject*>* coObjects) override;
    void Render() override;
    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
};
