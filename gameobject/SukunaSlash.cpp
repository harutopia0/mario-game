#include "SukunaSlash.h"
#include "../render/Sprites.h"
#include "../physics/Collision.h"
#include "Enemy.h"
#include "../audio/AudioManager.h"
#include <cstdlib>
#include <algorithm>

SukunaSlash::SukunaSlash(float x, float y, int direction) : GameObject(x, y) {
    this->direction = direction;
    this->vx = direction * 0.3f;
    this->vy = 0.0f;
    this->width = 24.0f;
    this->height = 24.0f;
    this->isSlashPhase = false;
    this->isShrinkPhase = false;
    this->targetEnemy = nullptr;
    this->hasDamaged = false;
    this->meleeCheckStartTime = GetTickCount64();
    this->startTime = 0;
    this->shrinkStartTime = 0;
    this->layer = LAYER_PLAYER;
}

void SukunaSlash::Update(DWORD dt, std::vector<GameObject*>* coObjects) {
    if (isDeleted) return;

    if (!isSlashPhase) {
        // --- PHASE 1: Invisible Melee Projectile ---
        // Exceeded 150ms without hitting anything -> self-destruct
        if (GetTickCount64() - meleeCheckStartTime > 150) {
            this->Delete();
            return;
        }

        float dx = vx * dt;
        float dy = vy * dt;

        float ml, mt, mr, mb;
        GetBoundingBox(ml, mt, mr, mb);

        for (UINT i = 0; i < coObjects->size(); i++) {
            GameObject* e = coObjects->at(i);
            if (e == this || e->IsDeleted()) continue;

            Enemy* enemy = dynamic_cast<Enemy*>(e);
            if (enemy && !enemy->IsDied() && !enemy->IsFreezed()) {
                float sl, st, sr, sb;
                enemy->GetBoundingBox(sl, st, sr, sb);

                // Check vertical overlapping
                if (mb > st && mt < sb) {
                    float t, temp_nx, temp_ny;
                    Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, dx, dy, sl, st, sr, sb, t, temp_nx, temp_ny);

                    bool isOverlap = (mr > sl && ml < sr);

                    if ((t < 1.0f) || isOverlap) {
                        // Switch to slash phase
                        isSlashPhase = true;
                        vx = 0.0f;
                        vy = 0.0f;
                        targetEnemy = enemy;
                        targetEnemy->SetFreezed(true);
                        startTime = GetTickCount64();

                        // Play slash sound effect
                        AudioManager::GetInstance()->PlaySFX("slash-sound");

                        // Pre-generate 6 slashes: 4 short (630) and 2 long (631) in random order
                        std::vector<int> spriteIds;
                        for (int k = 0; k < 4; k++) spriteIds.push_back(630);
                        for (int k = 0; k < 2; k++) spriteIds.push_back(631);

                        // Shuffle randomly using rand()
                        for (int k = 5; k > 0; k--) {
                            int idx = rand() % (k + 1);
                            std::swap(spriteIds[k], spriteIds[idx]);
                        }

                        // Determine monster size for offset
                        float ew = sr - sl;
                        float eh = sb - st;

                        // Generate random offset within the bounding box
                        for (int k = 0; k < 6; k++) {
                            SlashInstance slash;
                            slash.spriteId = spriteIds[k];

                            // Offset relative to center: random in [-ew/2, ew/2] and [-eh/2, eh/2]
                            float random_offset_x = (((float)rand() / RAND_MAX) * ew) - (ew / 2.0f);
                            float random_offset_y = (((float)rand() / RAND_MAX) * eh) - (eh / 2.0f);
                            slash.offsetX = random_offset_x;
                            slash.offsetY = random_offset_y;

                            // Random rotation from 0 to 2*PI radians
                            slash.rotation = (((float)rand() / RAND_MAX) * 2.0f * 3.14159265f);

                            // Shrink properties:
                            // Delay from shrink start: random 0 to 250ms
                            slash.shrinkDelay = (float)(rand() % 251);
                            // Duration of shrinking: random 200 to 350ms
                            slash.shrinkDuration = (float)(200 + rand() % 151);

                            slashes.push_back(slash);
                        }
                        break; // exit loop
                    }
                }
            }
        }

        x += dx;
        y += dy;
    } else {
        // --- PHASE 2: Slash Effect / Shrink Effect ---
        // If the target enemy was deleted or died by other means
        if (targetEnemy == nullptr || targetEnemy->IsDeleted()) {
            this->Delete();
            return;
        }

        if (!isShrinkPhase) {
            ULONGLONG elapsed = GetTickCount64() - startTime;

            // Damage target at 667ms (when all 6 slashes at 67ms finish spawning)
            if (elapsed >= 667 && !hasDamaged) {
                if (!targetEnemy->IsDied()) {
                    targetEnemy->SetDied(true);
                }
                hasDamaged = true;
                isShrinkPhase = true;
                shrinkStartTime = GetTickCount64();
            }
        } else {
            ULONGLONG shrinkElapsed = GetTickCount64() - shrinkStartTime;
            bool allFinished = true;
            for (int i = 0; i < 6; i++) {
                float t = (float)shrinkElapsed - slashes[i].shrinkDelay;
                if (t < slashes[i].shrinkDuration) {
                    allFinished = false;
                    break;
                }
            }
            if (allFinished) {
                this->Delete();
                return;
            }
        }
    }
}

void SukunaSlash::Render() {
    if (!isSlashPhase) return; // Invisible in projectile phase
    if (targetEnemy == nullptr || targetEnemy->IsDeleted()) return;

    // Get current target enemy bounding box/center
    float sl, st, sr, sb;
    targetEnemy->GetBoundingBox(sl, st, sr, sb);
    float enemyCenterX = sl + (sr - sl) / 2.0f;
    float enemyCenterY = st + (sb - st) / 2.0f;

    if (!isShrinkPhase) {
        ULONGLONG elapsed = GetTickCount64() - startTime;
        // Every 67ms (2/30 second), one more slash is drawn.
        // At t < 67ms -> 1st slash drawn (index 0).
        // At t = 335ms+ -> all 6 slashes drawn (indices 0 to 5).
        int maxIndex = (int)(elapsed / 67);
        if (maxIndex > 5) maxIndex = 5;

        for (int i = 0; i <= maxIndex; i++) {
            const SlashInstance& slash = slashes[i];
            Sprite* sprite = Sprites::GetInstance()->Get(slash.spriteId);
            if (sprite != nullptr) {
                int sw = sprite->GetWidth();
                int sh = sprite->GetHeight();
                float drawX = enemyCenterX + slash.offsetX - sw / 2.0f;
                float drawY = enemyCenterY + slash.offsetY - sh / 2.0f;
                sprite->DrawRotatedScaled(drawX, drawY, slash.rotation, 1.0f, 1.0f);
            }
        }
    } else {
        ULONGLONG shrinkElapsed = GetTickCount64() - shrinkStartTime;

        for (int i = 0; i < 6; i++) {
            const SlashInstance& slash = slashes[i];
            float t = (float)shrinkElapsed - slash.shrinkDelay;
            float scale = 1.0f;

            if (t > 0.0f) {
                if (t < slash.shrinkDuration) {
                    scale = 1.0f - (t / slash.shrinkDuration);
                } else {
                    scale = 0.0f; // completely shrunk, skip drawing
                }
            }

            if (scale > 0.0f) {
                Sprite* sprite = Sprites::GetInstance()->Get(slash.spriteId);
                if (sprite != nullptr) {
                    int sw = sprite->GetWidth();
                    int sh = sprite->GetHeight();
                    float drawX = enemyCenterX + slash.offsetX - sw / 2.0f;
                    float drawY = enemyCenterY + slash.offsetY - sh / 2.0f;
                    sprite->DrawRotatedScaled(drawX, drawY, slash.rotation, scale, 1.0f);
                }
            }
        }
    }
}

void SukunaSlash::GetBoundingBox(float& left, float& top, float& right, float& bottom) {
    if (!isSlashPhase) {
        left = x;
        top = y;
        right = x + width;
        bottom = y + height;
    } else {
        left = 0;
        top = 0;
        right = 0;
        bottom = 0;
    }
}
