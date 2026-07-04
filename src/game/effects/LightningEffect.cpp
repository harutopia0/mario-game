#include "game/effects/LightningEffect.h"
#include "engine/graphics/Sprites.h"
#include "game/scenes/SceneManager.h"
#include "engine/core/Camera.h"
#include <cmath>
#include <cstdlib>

LightningEffect::LightningEffect(float centerX, float centerY, int marioDir) : GameObject(centerX, centerY) {
    this->startTime = GetTickCount64();
    this->layer = LAYER_EFFECTS; // Render on top of everything
    this->marioDir = marioDir;
    this->currentPhase = 1;
    this->lastFlashIndex = -1;

    // Kích hoạt hit stop (100ms)
    SceneManager::GetInstance()->StartHitStop(100.0f);

    // Kích hoạt zoom camera 1.05x trong 100ms
    Camera::GetInstance()->Zoom(1.05f, 0.1f);

    // Phase 1 - Compression: Tạo 35-45 hạt màu đen co cụm quanh tâm va chạm
    int numParticles = 35 + (rand() % 11);
    for (int i = 0; i < numParticles; i++) {
        LightningParticle p;
        float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
        float radius = 30.0f + ((float)rand() / RAND_MAX) * 20.0f; // Bán kính 30-50px
        p.x = x + cos(angle) * radius;
        p.y = y + sin(angle) * radius;
        p.targetX = x;
        p.targetY = y;
        p.vx = 0.0f;
        p.vy = 0.0f;
        p.startSize = 2.0f + ((float)rand() / RAND_MAX) * 2.0f; // Kích cỡ 2-4px
        p.size = p.startSize;
        p.angle = angle;
        p.angularVelocity = 0.0f;
        p.color = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f); // Màu đen
        p.alpha = 1.0f;
        p.life = 50.0f; // 50ms
        p.maxLife = 50.0f;
        p.type = 1; // Compression
        p.initialDist = radius;

        particles.push_back(p);
    }
}

void LightningEffect::GenerateLightning() {
    lines.clear();

    // Tạo 4-6 tia sét ngắn ngẫu nhiên 360 độ từ tâm
    int numBranches = 4 + (rand() % 3);
    for (int b = 0; b < numBranches; b++) {
        float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
        float length = 12.0f + (float)(rand() % 15); // Độ dài ngắn 12-27px
        float targetX = x + cos(angle) * length;
        float targetY = y + sin(angle) * length;

        GenerateLightningPath(x, y, targetX, targetY);
    }
}

void LightningEffect::GenerateLightningPath(float x1, float y1, float x2, float y2) {
    std::vector<D3DXVECTOR2> points;
    points.push_back(D3DXVECTOR2(x1, y1));

    float dx = x2 - x1;
    float dy = y2 - y1;
    float dist = sqrt(dx * dx + dy * dy);
    if (dist < 3.0f) return;

    int numSteps = (int)(dist / 5.0f); // Phân đoạn ngắn 5px
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
            float sway = 2.5f; // Độ lệch nhỏ phù hợp với tia sét ngắn
            float offset = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * sway;
            targetX += px * offset;
            targetY += py * offset;
        }

        // Taper: Sét mảnh dần từ 1.0 về 0.2 ở ngọn
        float tSeg = (float)(i - 0.5f) / numSteps;
        float scale = 1.0f - tSeg * 0.8f;

        lines.push_back({prevX, prevY, targetX, targetY, scale});
        points.push_back(D3DXVECTOR2(targetX, targetY));
        prevX = targetX;
        prevY = targetY;
    }

    // Tạo nhánh phụ cho sét
    int size = (int)points.size();
    if (size > 3) {
        int numBranches = 1;
        for (int b = 0; b < numBranches; b++) {
            int idx = 1 + (rand() % (size - 2));
            D3DXVECTOR2 startPt = points[idx];

            float angle = atan2(y2 - y1, x2 - x1);
            float swayAngle = (((float)rand() / RAND_MAX) * 1.56f) - 0.78f; // +/- 45 độ
            angle += swayAngle;

            float length = 8.0f + (rand() % 8); // Nhánh phụ ngắn 8-16px
            float branchEndX = startPt.x + cos(angle) * length;
            float branchEndY = startPt.y + sin(angle) * length;

            float baseScale = 1.0f - ((float)idx / numSteps) * 0.8f;

            GenerateBranchPath(startPt.x, startPt.y, branchEndX, branchEndY, baseScale);
        }
    }
}

void LightningEffect::GenerateBranchPath(float bx1, float by1, float bx2, float by2, float baseScale) {
    float dx = bx2 - bx1;
    float dy = by2 - by1;
    float dist = sqrt(dx * dx + dy * dy);
    if (dist < 3.0f) return;

    int numSteps = (int)(dist / 4.0f);
    if (numSteps < 3) numSteps = 3;

    float px = -dy / dist;
    float py = dx / dist;

    float prevX = bx1;
    float prevY = by1;

    for (int i = 1; i <= numSteps; i++) {
        float t = (float)i / numSteps;
        float targetX = bx1 + dx * t;
        float targetY = by1 + dy * t;

        if (i < numSteps) {
            float sway = 2.0f;
            float offset = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * sway;
            targetX += px * offset;
            targetY += py * offset;
        }

        float tSeg = (float)(i - 0.5f) / numSteps;
        float scale = baseScale * (1.0f - tSeg * 0.8f);

        lines.push_back({prevX, prevY, targetX, targetY, scale});
        prevX = targetX;
        prevY = targetY;
    }
}

void LightningEffect::Update(DWORD dt, std::vector<GameObject*>* coObjects) {
    if (isDeleted) return;

    ULONGLONG elapsed = GetTickCount64() - startTime;

    // Quản lý chuyển phase theo trục thời gian
    int newPhase = 1;
    if (elapsed >= 240) {
        newPhase = 4;
    } else if (elapsed >= 90) {
        newPhase = 3;
    } else if (elapsed >= 50) {
        newPhase = 2;
    }

    if (newPhase != currentPhase) {
        currentPhase = newPhase;

        if (currentPhase == 2) {
            // Rung màn hình: Cường độ mạnh 6px, giảm dần trong 0.2 giây
            Camera::GetInstance()->Shake(6.0f, 0.2f);
            
            // Tạo các tia sét đỏ đen ngẫu nhiên 360 độ từ tâm
            GenerateLightning();
        }
        else if (currentPhase == 3) {
            // Spawn 10-15 mảnh vỡ không gian (đa giác bất quy tắc)
            int numShards = 10 + (rand() % 6);
            for (int i = 0; i < numShards; i++) {
                SpaceShard shard;
                shard.x = x;
                shard.y = y;

                float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
                float speed = 60.0f + ((float)rand() / RAND_MAX) * 80.0f; // px/s
                shard.vx = cos(angle) * speed / 1000.0f;
                shard.vy = sin(angle) * speed / 1000.0f;

                shard.angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
                shard.angularVelocity = (-3.14f + ((float)rand() / RAND_MAX) * 6.28f) / 1000.0f;
                shard.scale = 1.0f;
                shard.life = 150.0f;
                shard.maxLife = 150.0f;

                // Màu đen hoặc đỏ sẫm (#7A0000)
                if (rand() % 2 == 0) {
                    shard.color = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
                } else {
                    shard.color = D3DXCOLOR(0.48f, 0.0f, 0.0f, 1.0f);
                }

                // Điểm đỉnh cục bộ tạo thành đa giác méo mó (3-5 đỉnh, bán kính 4-8px)
                int numVertices = 3 + (rand() % 3);
                for (int v = 0; v < numVertices; v++) {
                    float vAngle = ((float)v / numVertices) * 2.0f * 3.14159265f + (((float)rand() / RAND_MAX) * 0.5f - 0.25f);
                    float vRadius = 4.0f + ((float)rand() / RAND_MAX) * 4.0f;
                    shard.localPoints.push_back(D3DXVECTOR2(cos(vAngle) * vRadius, sin(vAngle) * vRadius));
                }

                shards.push_back(shard);
            }

            // Spawn 4-6 vệt chém dẹt mỏng màu đỏ sẫm bay theo hướng đòn đánh của Mario
            int numSlashes = 4 + (rand() % 3);
            for (int i = 0; i < numSlashes; i++) {
                LightningParticle p;
                p.x = x;
                p.y = y;

                float baseAngle = (marioDir > 0) ? 0.0f : 3.14159265f;
                float angleSpread = -0.4f + ((float)rand() / RAND_MAX) * 0.8f; // Trải rộng góc +/- 23 độ
                float angle = baseAngle + angleSpread;

                float speed = 160.0f + ((float)rand() / RAND_MAX) * 120.0f;
                p.vx = cos(angle) * speed / 1000.0f;
                p.vy = sin(angle) * speed / 1000.0f;

                p.startSize = 16.0f + ((float)rand() / RAND_MAX) * 16.0f; // Độ dài vệt chém
                p.size = p.startSize;
                p.angle = angle;
                p.angularVelocity = 0.0f;
                p.color = D3DXCOLOR(0.69f, 0.0f, 0.0f, 1.0f); // Crimson đỏ thẫm (#B00000)
                p.alpha = 1.0f;
                p.life = 150.0f;
                p.maxLife = 150.0f;
                p.type = 3; // Slash

                particles.push_back(p);
            }
        }
        else if (currentPhase == 4) {
            // Spawn 20-30 bụi không gian màu đen
            int numDust = 20 + (rand() % 11);
            for (int i = 0; i < numDust; i++) {
                LightningParticle p;
                float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
                float radius = ((float)rand() / RAND_MAX) * 18.0f;
                p.x = x + cos(angle) * radius;
                p.y = y + sin(angle) * radius;

                float dAngle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
                float speed = 6.0f + ((float)rand() / RAND_MAX) * 12.0f; // Trôi chậm
                p.vx = cos(dAngle) * speed / 1000.0f;
                p.vy = sin(dAngle) * speed / 1000.0f;

                p.startSize = 1.0f + ((float)rand() / RAND_MAX) * 1.5f;
                p.size = p.startSize;
                p.angle = dAngle;
                p.angularVelocity = 0.0f;
                p.color = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f); // Màu đen
                p.alpha = 0.8f;
                p.life = 200.0f;
                p.maxLife = 200.0f;
                p.type = 4; // Dust

                particles.push_back(p);
            }
        }
    }

    // Cập nhật tọa độ và kích thước các thực thể
    if (currentPhase == 1) {
        for (auto& p : particles) {
            float dx = p.targetX - p.x;
            float dy = p.targetY - p.y;
            float dist = sqrt(dx * dx + dy * dy);

            if (dist > 1.0f) {
                float speed = 0.12f + (p.initialDist - dist) * 0.012f; // Tăng tốc khi tiến gần
                p.vx = dx / dist * speed;
                p.vy = dy / dist * speed;

                p.x += p.vx * dt;
                p.y += p.vy * dt;

                p.size = p.startSize * (dist / p.initialDist); // Thu nhỏ dần
                if (p.size < 0.2f) p.size = 0.2f;
            } else {
                p.x = p.targetX;
                p.y = p.targetY;
                p.size = 0.0f;
            }
        }
    }
    else if (currentPhase == 2) {
        // Thay đổi ngẫu nhiên hình dạng tia sét trong Phase 2 chớp sáng
        int currentFlashIndex = (int)((elapsed - 50) / 15);
        if (currentFlashIndex % 2 == 0) {
            if (currentFlashIndex != lastFlashIndex) {
                lastFlashIndex = currentFlashIndex;
                GenerateLightning();
            }
        }
    }
    else if (currentPhase == 3) {
        // Mảnh vỡ không gian bay ra và thu nhỏ dần
        for (auto& shard : shards) {
            shard.x += shard.vx * dt;
            shard.y += shard.vy * dt;
            shard.angle += shard.angularVelocity * dt;
            shard.life -= (float)dt;
            if (shard.life < 0) shard.life = 0;
            shard.scale = shard.life / shard.maxLife;
        }

        // Vệt chém phai mờ
        for (auto& p : particles) {
            if (p.type == 3) {
                p.x += p.vx * dt;
                p.y += p.vy * dt;
                p.life -= (float)dt;
                if (p.life < 0) p.life = 0;
                p.alpha = p.life / p.maxLife;
            }
        }
    }
    else if (currentPhase == 4) {
        // Bụi mờ dần trôi chậm
        for (auto& p : particles) {
            if (p.type == 4) {
                p.x += p.vx * dt;
                p.y += p.vy * dt;
                p.life -= (float)dt;
                if (p.life < 0) p.life = 0;
                p.alpha = (p.life / p.maxLife) * 0.8f;
            }
        }
    }

    // Tự hủy sau 450ms
    if (elapsed > 450) {
        Delete();
    }
}

void LightningEffect::Render() {
    ULONGLONG elapsed = GetTickCount64() - startTime;

    Sprite* whiteSprite = Sprites::GetInstance()->Get(99998);
    if (whiteSprite == nullptr) return;

    if (currentPhase == 1) {
        // Vẽ hạt nén màu đen
        for (const auto& p : particles) {
            if (p.type == 1 && p.size > 0.0f) {
                whiteSprite->DrawRotatedScaled(p.x, p.y, p.angle, p.size, p.size, p.color);
            }
        }
    }
    else if (currentPhase == 2) {
        float phase2Time = (float)(elapsed - 50);

        // Nháy trắng màn hình trong 30ms đầu Phase 2
        if (phase2Time < 30.0f) {
            float flashAlpha = 0.8f * (1.0f - phase2Time / 30.0f);
            Sprite* blackOverlay = Sprites::GetInstance()->Get(99999);
            if (blackOverlay) {
                float camX = Camera::GetInstance()->GetX();
                float camY = Camera::GetInstance()->GetY();
                blackOverlay->Draw(camX, camY, 320.0f, 240.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, flashAlpha));
            }
        }

        // Vẽ vòng tròn đỏ đen nở ra từ tâm
        float circleScale = phase2Time / 40.0f;
        float maxRadius = 30.0f;
        float currentRadius = circleScale * maxRadius;
        float ringAlpha = 1.0f - circleScale;

        if (ringAlpha > 0.0f && currentRadius > 1.0f) {
            int segments = 16;
            float prevX = x + currentRadius;
            float prevY = y;
            D3DXCOLOR ringColor = (rand() % 2 == 0) ? D3DXCOLOR(0.0f, 0.0f, 0.0f, ringAlpha) : D3DXCOLOR(0.69f, 0.0f, 0.0f, ringAlpha);

            for (int s = 1; s <= segments; s++) {
                float angle = ((float)s / segments) * 2.0f * 3.14159265f;
                float currX = x + cos(angle) * currentRadius;
                float currY = y + sin(angle) * currentRadius;

                float dx = currX - prevX;
                float dy = currY - prevY;
                float len = sqrt(dx * dx + dy * dy);
                float segAngle = atan2(dy, dx);
                float cx = (prevX + currX) / 2.0f;
                float cy = (prevY + currY) / 2.0f;

                whiteSprite->DrawRotatedScaled(cx - 0.5f, cy - 0.5f, segAngle, len, 2.0f, ringColor);

                prevX = currX;
                prevY = currY;
            }
        }

        // Vẽ các tia sét ngắn
        int flashIndex = (int)(phase2Time / 15);
        if (flashIndex % 2 == 0) {
            // Nét đen viền ngoài (3px)
            for (const auto& line : lines) {
                float dx = line.x2 - line.x1;
                float dy = line.y2 - line.y1;
                float len = sqrt(dx * dx + dy * dy);
                float angle = atan2(dy, dx);
                float cx = (line.x1 + line.x2) / 2.0f;
                float cy = (line.y1 + line.y2) / 2.0f;

                whiteSprite->DrawRotatedScaled(cx - 0.5f, cy - 0.5f, angle, len, 3.0f * line.scale, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
            }

            // Nét đỏ sẫm trung gian (1.8px)
            for (const auto& line : lines) {
                float dx = line.x2 - line.x1;
                float dy = line.y2 - line.y1;
                float len = sqrt(dx * dx + dy * dy);
                float angle = atan2(dy, dx);
                float cx = (line.x1 + line.x2) / 2.0f;
                float cy = (line.y1 + line.y2) / 2.0f;

                whiteSprite->DrawRotatedScaled(cx - 0.5f, cy - 0.5f, angle, len, 1.8f * line.scale, D3DXCOLOR(0.6f, 0.0f, 0.0f, 1.0f));
            }

            // Nét trắng hồng lõi sáng (0.8px)
            for (const auto& line : lines) {
                float dx = line.x2 - line.x1;
                float dy = line.y2 - line.y1;
                float len = sqrt(dx * dx + dy * dy);
                float angle = atan2(dy, dx);
                float cx = (line.x1 + line.x2) / 2.0f;
                float cy = (line.y1 + line.y2) / 2.0f;

                whiteSprite->DrawRotatedScaled(cx - 0.5f, cy - 0.5f, angle, len, 0.8f * line.scale, D3DXCOLOR(1.0f, 0.9f, 0.9f, 1.0f));
            }
        }
    }
    else if (currentPhase == 3) {
        // Vẽ mảnh vỡ không gian đa giác dạng nét rỗng (wireframe)
        for (const auto& shard : shards) {
            if (shard.life > 0 && !shard.localPoints.empty()) {
                int numPts = (int)shard.localPoints.size();
                std::vector<D3DXVECTOR2> worldPts;
                float s = sin(shard.angle);
                float c = cos(shard.angle);

                for (int v = 0; v < numPts; v++) {
                    float localX = shard.localPoints[v].x * shard.scale;
                    float localY = shard.localPoints[v].y * shard.scale;
                    float worldX = (localX * c - localY * s) + shard.x;
                    float worldY = (localX * s + localY * c) + shard.y;
                    worldPts.push_back(D3DXVECTOR2(worldX, worldY));
                }

                for (int v = 0; v < numPts; v++) {
                    D3DXVECTOR2 p1 = worldPts[v];
                    D3DXVECTOR2 p2 = worldPts[(v + 1) % numPts];

                    float dx = p2.x - p1.x;
                    float dy = p2.y - p1.y;
                    float len = sqrt(dx * dx + dy * dy);
                    float angle = atan2(dy, dx);
                    float cx = (p1.x + p2.x) / 2.0f;
                    float cy = (p1.y + p2.y) / 2.0f;

                    whiteSprite->DrawRotatedScaled(cx - 0.5f, cy - 0.5f, angle, len, 1.2f, shard.color);
                }
            }
        }

        // Vẽ vệt chém kéo dài dẹt mỏng
        for (const auto& p : particles) {
            if (p.type == 3 && p.life > 0.0f) {
                D3DXCOLOR c = p.color;
                c.a = p.alpha;
                whiteSprite->DrawRotatedScaled(p.x - 0.5f, p.y - 0.5f, p.angle, p.size, 1.2f, c);
            }
        }
    }
    else if (currentPhase == 4) {
        // Vẽ bụi không gian đen mờ dần
        for (const auto& p : particles) {
            if (p.type == 4 && p.life > 0.0f) {
                D3DXCOLOR c = p.color;
                c.a = p.alpha;
                whiteSprite->DrawRotatedScaled(p.x, p.y, p.angle, p.size, p.size, c);
            }
        }
    }
}

void LightningEffect::GetBoundingBox(float& left, float& top, float& right, float& bottom) {
    left = 0;
    top = 0;
    right = 0;
    bottom = 0;
}
