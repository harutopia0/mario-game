#include "game/projectiles/EnemyFireball.h"
#include "engine/graphics/Animations.h"
#include "engine/core/Collision.h"
#include "game/entities/Mario.h"
#include "game/entities/Enemy.h"
#include "game/objects/Block.h"

EnemyFireball::EnemyFireball(float x, float y, float vx, float vy) : Projectile(x, y, 1) {
    this->width = ENEMY_FIREBALL_WIDTH;
    this->height = ENEMY_FIREBALL_HEIGHT;
    this->animationId = 600; // Dùng chung animation đạn của Mario
    this->vx = vx;
    this->vy = vy;
}

void EnemyFireball::Update(DWORD dt, vector<GameObject*>* coObjects) {
    if (isDeleted) return;

    float dx = vx * dt;
    float dy = vy * dt;

    // Check collision
    float ml, mt, mr, mb;
    GetBoundingBox(ml, mt, mr, mb);

    if (isParried) {
        for (UINT i = 0; i < coObjects->size(); i++) {
            GameObject* e = coObjects->at(i);
            if (e == this || e->IsDeleted()) continue;

            float sl, st, sr, sb;
            e->GetBoundingBox(sl, st, sr, sb);

            // AABB check
            if (!(mr < sl || ml > sr || mb < st || mt > sb)) {
                if (Enemy* enemy = dynamic_cast<Enemy*>(e)) {
                    if (!enemy->IsDied()) {
                        enemy->OnStomped(NULL);
                        this->Delete();
                        return;
                    }
                }
                else if (Block* block = dynamic_cast<Block*>(e)) {
                    if (!block->IsOneWay()) {
                        this->Delete();
                        return;
                    }
                }
            }
        }
    }
    else {
        for (UINT i = 0; i < coObjects->size(); i++) {
            GameObject* e = coObjects->at(i);
            if (e == this || e->IsDeleted()) continue;

            if (Mario* mario = dynamic_cast<Mario*>(e)) {
                float sl, st, sr, sb;
                mario->GetBoundingBox(sl, st, sr, sb);

                // AABB check
                if (!(mr < sl || ml > sr || mb < st || mt > sb)) {
                    mario->TakeDamage();
                    this->Delete();
                    return;
                }
            }
        }
    }

    x += dx;
    y += dy;
}

void EnemyFireball::Render() {
    if (isDeleted) return;
    Animation* anim = Animations::GetInstance()->Get(animationId);
    if (anim != NULL) {
        anim->Render(x, y);
    }
}
