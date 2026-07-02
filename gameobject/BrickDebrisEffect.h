#pragma once
#include "../core/GameObject.h"

// Quản lý 4 mảnh vụn gạch bay ra khi Breakable bị phá vỡ
class BrickDebrisEffect : public GameObject {
private:
    struct DebrisPiece {
        float x, y;
        float vx, vy;
    };
    DebrisPiece pieces[4];
    int animationId;

public:
    BrickDebrisEffect(float x, float y, int animationId);
    virtual void Update(DWORD dt, vector<GameObject*>* coObjects) override;
    virtual void Render() override;
    virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
};
