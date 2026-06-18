#include "LightningEffect.h"
#include "../render/Sprites.h"
#include <cmath>
#include <cstdlib>

LightningEffect::LightningEffect(float centerX, float centerY) : GameObject(centerX, centerY) {
    this->startTime = GetTickCount64();
    this->layer = LAYER_EFFECTS; // Render on top of everything

    // Generate 5-7 lightning branches radiating outwards from the monster
    int numBranches = 5 + (rand() % 3);
    for (int b = 0; b < numBranches; b++) {
        float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
        float length = 35.0f + (float)(rand() % 25); // 35-60px branch length
        float targetX = centerX + cos(angle) * length;
        float targetY = centerY + sin(angle) * length;

        GenerateLightningPath(centerX, centerY, targetX, targetY);
    }
}

void LightningEffect::GenerateLightningPath(float x1, float y1, float x2, float y2) {
    std::vector<D3DXVECTOR2> points;
    points.push_back(D3DXVECTOR2(x1, y1));

    float dx = x2 - x1;
    float dy = y2 - y1;
    float dist = sqrt(dx * dx + dy * dy);
    if (dist < 5.0f) return;

    int numSteps = (int)(dist / 8.0f); // segment every 8 pixels
    if (numSteps < 3) numSteps = 3;

    float px = -dy / dist;
    float py = dx / dist;

    float prevX = x1;
    float prevY = y1;

    for (int i = 1; i <= numSteps; i++) {
        float t = (float)i / numSteps;
        float targetX = x1 + dx * t;
        float targetY = y1 + dy * t;

        if (i < numSteps) {
            float sway = 4.0f; // maximum noise offset
            float offset = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * sway;
            targetX += px * offset;
            targetY += py * offset;
        }

        lines.push_back({prevX, prevY, targetX, targetY});
        points.push_back(D3DXVECTOR2(targetX, targetY));
        prevX = targetX;
        prevY = targetY;
    }

    // Spawn 1 or 2 sub-branches along the main branch
    int size = (int)points.size();
    if (size > 4) {
        int numBranches = 1 + (rand() % 2);
        for (int b = 0; b < numBranches; b++) {
            int idx = 1 + (rand() % (size - 2));
            D3DXVECTOR2 startPt = points[idx];

            float angle = atan2(y2 - y1, x2 - x1);
            float swayAngle = (((float)rand() / RAND_MAX) * 1.56f) - 0.78f; // +/- 45 deg
            angle += swayAngle;

            float length = 15.0f + (rand() % 15); // 15-30px branch length
            float branchEndX = startPt.x + cos(angle) * length;
            float branchEndY = startPt.y + sin(angle) * length;

            GenerateBranchPath(startPt.x, startPt.y, branchEndX, branchEndY, 1);
        }
    }
}

void LightningEffect::GenerateBranchPath(float bx1, float by1, float bx2, float by2, int depth) {
    float dx = bx2 - bx1;
    float dy = by2 - by1;
    float dist = sqrt(dx * dx + dy * dy);
    if (dist < 4.0f) return;

    int numSteps = (int)(dist / 6.0f);
    if (numSteps < 3) numSteps = 3;

    float px = -dy / dist;
    float py = dx / dist;

    float prevX = bx1;
    float prevY = by1;

    std::vector<D3DXVECTOR2> branchPoints;

    for (int i = 1; i <= numSteps; i++) {
        float t = (float)i / numSteps;
        float targetX = bx1 + dx * t;
        float targetY = by1 + dy * t;

        if (i < numSteps) {
            float sway = 3.0f;
            float offset = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * sway;
            targetX += px * offset;
            targetY += py * offset;
        }

        lines.push_back({prevX, prevY, targetX, targetY});
        branchPoints.push_back(D3DXVECTOR2(targetX, targetY));
        prevX = targetX;
        prevY = targetY;
    }
}

void LightningEffect::Update(DWORD dt, std::vector<GameObject*>* coObjects) {
    if (isDeleted) return;

    ULONGLONG elapsed = GetTickCount64() - startTime;
    if (elapsed > 300) { // Self-destruct after 300ms
        Delete();
    }
}

void LightningEffect::Render() {
    ULONGLONG elapsed = GetTickCount64() - startTime;

    // Lightning discharge flickering:
    // Every 45ms we switch between rendering and not rendering
    if ((elapsed / 45) % 2 == 0) {
        Sprite* whiteSprite = Sprites::GetInstance()->Get(99998);
        if (whiteSprite == nullptr) return;

        // Pass 1: Black outer border (3.0f)
        for (const auto& line : lines) {
            float dx = line.x2 - line.x1;
            float dy = line.y2 - line.y1;
            float len = sqrt(dx * dx + dy * dy);
            float angle = atan2(dy, dx);
            float cx = (line.x1 + line.x2) / 2.0f;
            float cy = (line.y1 + line.y2) / 2.0f;

            whiteSprite->DrawRotatedScaled(cx - 0.5f, cy - 0.5f, angle, len, 3.0f, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
        }

        // Pass 2: Dark red glow layer (1.8f)
        for (const auto& line : lines) {
            float dx = line.x2 - line.x1;
            float dy = line.y2 - line.y1;
            float len = sqrt(dx * dx + dy * dy);
            float angle = atan2(dy, dx);
            float cx = (line.x1 + line.x2) / 2.0f;
            float cy = (line.y1 + line.y2) / 2.0f;

            whiteSprite->DrawRotatedScaled(cx - 0.5f, cy - 0.5f, angle, len, 1.8f, D3DXCOLOR(0.6f, 0.0f, 0.0f, 1.0f));
        }

        // Pass 3: Inner bright white/pink core (0.8f)
        for (const auto& line : lines) {
            float dx = line.x2 - line.x1;
            float dy = line.y2 - line.y1;
            float len = sqrt(dx * dx + dy * dy);
            float angle = atan2(dy, dx);
            float cx = (line.x1 + line.x2) / 2.0f;
            float cy = (line.y1 + line.y2) / 2.0f;

            whiteSprite->DrawRotatedScaled(cx - 0.5f, cy - 0.5f, angle, len, 0.8f, D3DXCOLOR(1.0f, 0.9f, 0.9f, 1.0f));
        }
    }
}

void LightningEffect::GetBoundingBox(float& left, float& top, float& right, float& bottom) {
    // Non-collidable visual effect
    left = 0;
    top = 0;
    right = 0;
    bottom = 0;
}
