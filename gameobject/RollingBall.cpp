#include "RollingBall.h"
#include "../gameplay/Map.h"
#include "../animation/Animations.h"
#include "../physics/Collision.h"
#include "../gameobject/Platform.h"
#include "../gameobject/Enemy.h"
#include "../gameobject/Block.h"

RollingBall::RollingBall(float x, float y, int direction) : Projectile(x, y, direction) {
    width = ROLLINGBALL_WIDTH;
    height = ROLLINGBALL_HEIGHT;
    animationId = 620; // Rolling Ball animation
    vx = direction * ROLLINGBALL_SPEED_X;
    vy = 0;
    state = ROLLINGBALL_STATE_NORMAL;
    explodeStart = 0;
    rotationAngle = 0.0f;
}

void RollingBall::Explode() {
    state = ROLLINGBALL_STATE_EXPLODING;
    explodeStart = GetTickCount64();
    animationId = 605; // Fireball explosion animation used here too
}

void RollingBall::Update(DWORD dt, vector<GameObject*>* coObjects) {
    if (isDeleted) return;

    if (state == ROLLINGBALL_STATE_EXPLODING) {
        vx = 0;
        vy = 0;
        if (GetTickCount64() - explodeStart > 150) {
            this->Delete();
        }
        return;
    }

    if (y < 0.0f) {
        this->Delete();
        return;
    }

    auto& g_objectList = Map::GetInstance()->GetObjects();
    if (!g_objectList.empty() && g_objectList[0] != nullptr) {
        float marioX = g_objectList[0]->GetX();
        // Camera (màn hình) rộng khoảng 320px (khi zoom 2x), nếu cách Mario > 350px là ngoài tầm camera.
        if (std::abs(this->x - marioX) > 350.0f) {
            this->Delete();
            return;
        }
    }

    vy += ROLLINGBALL_GRAVITY * dt;

    float dx = vx * dt;
    float dy = vy * dt;
    
    rotationAngle -= dx * 0.0588f; // C = pi * d = 3.14 * 34 = 106.76. rad/px = 2*PI / 106.76 = 0.0588

    // QUÉT VA CHẠM TRỤC X
    float min_tx = 1.0f;
    float nx_col = 0;
    float ml, mt, mr, mb;
    GetBoundingBox(ml, mt, mr, mb);

    for (UINT i = 0; i < coObjects->size(); i++) {
        GameObject* e = coObjects->at(i);
        if (e == this || e->IsDeleted()) continue;

        float sl, st, sr, sb;
        e->GetBoundingBox(sl, st, sr, sb);

        if (mb > st && mt < sb) {
            float t, temp_nx, temp_ny;
            Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, dx, 0.0f, sl, st, sr, sb, t, temp_nx, temp_ny);

            if (t < 1.0f && temp_nx != 0) {
                if (Enemy* enemy = dynamic_cast<Enemy*>(e)) {
                    if (!enemy->IsDied()) {
                        enemy->SetDied(true);
                    }
                }
                else if (dynamic_cast<Block*>(e) && !e->IsOneWay()) {
                    if (t < min_tx) {
                        min_tx = t;
                        nx_col = temp_nx;
                    }
                }
            }
        }
    }

    x += min_tx * dx + nx_col * 0.01f;
    if (nx_col != 0) {
        vx = -vx; // Dội ngược lại mãi mãi, không bị vỡ sau 3 lần nữa
    }

    // QUÉT VA CHẠM TRỤC Y
    GetBoundingBox(ml, mt, mr, mb);
    float min_ty = 1.0f;
    float ny_col = 0;

    for (UINT i = 0; i < coObjects->size(); i++) {
        GameObject* e = coObjects->at(i);
        if (e == this || e->IsDeleted()) continue;

        float sl, st, sr, sb;
        e->GetBoundingBox(sl, st, sr, sb);

        if (mr > sl && ml < sr) {
            float t, temp_nx, temp_ny;
            Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, 0.0f, dy, sl, st, sr, sb, t, temp_nx, temp_ny);

            if (t < 1.0f && temp_ny != 0) {
                if (Enemy* enemy = dynamic_cast<Enemy*>(e)) {
                    // Không tiêu diệt quái vật khi đè lên để tránh lỗi collision nếu cần, nhưng cứ để
                    if (!enemy->IsDied()) {
                        enemy->SetDied(true);
                    }
                }
                else if (dynamic_cast<Block*>(e)) {
                    if (e->IsOneWay() && temp_ny != 1) continue;

                    if (t < min_ty) {
                        min_ty = t;
                        ny_col = temp_ny;
                    }
                }
            }
        }
    }

    y += min_ty * dy + ny_col * 0.01f;

    if (ny_col != 0) {
        if (ny_col == 1) { // Chạm đất
            vy = 0;
        } else { // Đập đầu lên trần
            vy = 0;
        }
    }
}

void RollingBall::Render() {
    Animation* ani = Animations::GetInstance()->Get(animationId);
    if (ani != NULL) {
        if (state == ROLLINGBALL_STATE_EXPLODING) {
            float offsetX = (ani->GetWidth() - ROLLINGBALL_WIDTH) / 2.0f;
            float offsetY = (ani->GetHeight() - ROLLINGBALL_HEIGHT) / 2.0f;
            ani->Render(x - offsetX, y - offsetY);
        } else {
            ani->Render(x, y, rotationAngle);
        }
    }
}
