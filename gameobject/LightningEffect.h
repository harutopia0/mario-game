#pragma once
#include "../core/GameObject.h"
#include <vector>
#include <d3d10.h>
#include <d3dx10.h>

struct LightningLine {
    float x1, y1;
    float x2, y2;
};

class LightningEffect : public GameObject {
private:
    ULONGLONG startTime;
    std::vector<LightningLine> lines;

    void GenerateLightningPath(float x1, float y1, float x2, float y2);
    void GenerateBranchPath(float bx1, float by1, float bx2, float by2, int depth);

public:
    LightningEffect(float centerX, float centerY);
    void Update(DWORD dt, std::vector<GameObject*>* coObjects) override;
    void Render() override;
    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
};
