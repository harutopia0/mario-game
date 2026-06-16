#include "Fireball.h"
#include "../animation/Animations.h"
#include "../physics/Collision.h"
#include "../gameobject/Brick.h"
#include "../gameobject/Pipe.h"
#include "../gameobject/Breakable.h"
#include "../gameobject/LuckyBlock.h"
#include "../gameobject/Platform.h"
#include "../gameobject/Enemy.h"

Fireball::Fireball(float x, float y, int direction) : Projectile(x, y, direction) {
    width = FIREBALL_WIDTH;
    height = FIREBALL_HEIGHT;
    animationId = 600; // Fireball animation
    vx = direction * FIREBALL_SPEED_X;
    vy = 0;
    state = FIREBALL_STATE_NORMAL;
    explodeStart = 0;
}

void Fireball::Update(DWORD dt, vector<GameObject*>* coObjects) {
    if (isDeleted) return;

    if (state == FIREBALL_STATE_EXPLODING) {
        vx = 0;
        vy = 0;
        if (GetTickCount64() - explodeStart > 150) {
            this->Delete();
        }
        return;
    }

    vy += FIREBALL_GRAVITY * dt;

    float dx = vx * dt;
    float dy = vy * dt;

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
                        x += t * dx; // Cập nhật vị trí tới sát quái
                        state = FIREBALL_STATE_EXPLODING;
                        layer = LAYER_EFFECTS;
                        explodeStart = GetTickCount64();
                        animationId = 605; // Explosion animation
                        return;
                    }
                }
                else if (dynamic_cast<Brick*>(e) || dynamic_cast<Pipe*>(e) || dynamic_cast<Breakable*>(e) || dynamic_cast<LuckyBlock*>(e)) {
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
        state = FIREBALL_STATE_EXPLODING;
        layer = LAYER_EFFECTS;
        explodeStart = GetTickCount64();
        animationId = 605; // Explosion animation
        return;
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
                    if (!enemy->IsDied()) {
                        enemy->SetDied(true);
                        y += t * dy; // Cập nhật vị trí tới sát quái
                        state = FIREBALL_STATE_EXPLODING;
                        layer = LAYER_EFFECTS;
                        explodeStart = GetTickCount64();
                        animationId = 605; // Explosion animation
                        return;
                    }
                }
                else if (dynamic_cast<Brick*>(e) || dynamic_cast<Pipe*>(e) || dynamic_cast<Breakable*>(e) || dynamic_cast<LuckyBlock*>(e) || dynamic_cast<Platform*>(e)) {
                    if (dynamic_cast<Platform*>(e) && temp_ny != 1) continue;

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
        if (ny_col == 1) { // Chạm đất thì nảy lên
            vy = FIREBALL_BOUNCE_SPEED;
        } else { // Đập đầu lên trần
            vy = 0;
        }
    }
}

void Fireball::Render() {
    Animation* ani = Animations::GetInstance()->Get(animationId);
    if (ani != NULL) {
        if (state == FIREBALL_STATE_EXPLODING) {
            float offsetX = (ani->GetWidth() - FIREBALL_WIDTH) / 2.0f;
            float offsetY = (ani->GetHeight() - FIREBALL_HEIGHT) / 2.0f;
            ani->Render(x - offsetX, y - offsetY);
        } else {
            ani->Render(x, y);
        }
    }
}
