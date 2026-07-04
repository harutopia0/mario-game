#pragma once
#include "engine/core/GameObject.h"
#include <vector>
#include <d3d10.h>
#include <d3dx10.h>

struct LightningLine {
    float x1, y1;
    float x2, y2;
    float scale;
};

struct LightningParticle {
    float x, y;
    float vx, vy;
    float size;
    float startSize;
    float angle;
    float angularVelocity;
    D3DXCOLOR color;
    float alpha;
    float life; // in milliseconds
    float maxLife;
    int type; // 1 = Compression, 3 = Slash, 4 = Dust
    float initialDist;
    float targetX, targetY;
};

struct SpaceShard {
    float x, y;
    float vx, vy;
    float angle;
    float angularVelocity;
    float scale;
    float life;
    float maxLife;
    D3DXCOLOR color;
    std::vector<D3DXVECTOR2> localPoints;
};

class LightningEffect : public GameObject {
private:
    ULONGLONG startTime;
    int marioDir;

    // Phase 2 lightning lines
    std::vector<LightningLine> lines;
    int lastFlashIndex;

    // Particles
    std::vector<LightningParticle> particles;

    // Space Shards
    std::vector<SpaceShard> shards;

    // Giai đoạn hiện tại
    int currentPhase;

    void GenerateLightning();
    void GenerateLightningPath(float x1, float y1, float x2, float y2);
    void GenerateBranchPath(float bx1, float by1, float bx2, float by2, float baseScale);

public:
    LightningEffect(float centerX, float centerY, int marioDir);
    void Update(DWORD dt, std::vector<GameObject*>* coObjects) override;
    void Render() override;
    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
};
