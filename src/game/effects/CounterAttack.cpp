#include "game/effects/CounterAttack.h"
#include "game/entities/Enemy.h"
#include "game/entities/Mario.h"
#include "game/projectiles/EnemyFireball.h"
#include "game/projectiles/Hammer.h"
#include "game/projectiles/Projectile.h"

CounterAttack::CounterAttack(Mario *mario) : GameObject()
{
    this->mario = mario;
    this->startTime = GetTickCount64();
    this->width = 20.0f;
    this->height = 30.0f;
    this->layer = LAYER_PLAYER;
}

void CounterAttack::Update(DWORD dt, std::vector<GameObject *> *coObjects)
{
    if (isDeleted)
        return;

    if (mario == nullptr || mario->IsDeleted() || !mario->IsParrying())
    {
        Delete();
        return;
    }

    // Sync position with Mario
    float ml, mt, mr, mb;
    mario->GetBoundingBox(ml, mt, mr, mb);

    // Position the parry hitbox to wrap Mario
    x = ml - 2.0f;
    y = mt - 2.0f;
    width = (mr - ml) + 4.0f;
    height = (mb - mt) + 4.0f;

    ULONGLONG elapsed = GetTickCount64() - startTime;
    if (elapsed > 250)
    { // 0.25s parry window
        mario->OnParryFailed();
        Delete();
        return;
    }

    // Check for overlap with any active enemy or target projectiles
    float myL, myT, myR, myB;
    GetBoundingBox(myL, myT, myR, myB);

    for (size_t i = 0; i < coObjects->size(); i++)
    {
        GameObject *obj = coObjects->at(i);
        if (obj == this || obj->IsDeleted())
            continue;

        Enemy *enemy = dynamic_cast<Enemy *>(obj);
        if (enemy && !enemy->IsDied() && !enemy->IsFreezed())
        {
            float el, et, er, eb;
            enemy->GetBoundingBox(el, et, er, eb);

            // AABB overlap check
            if (myR > el && myL < er && myB > et && myT < eb)
            {
                mario->OnParrySuccess(enemy);
                Delete();
                return;
            }
        }

        Projectile *proj = dynamic_cast<Projectile *>(obj);
        if (proj && !proj->IsDeleted() && !proj->IsParried())
        {
            bool isTargetProjectile = (dynamic_cast<EnemyFireball *>(proj) != nullptr) || (dynamic_cast<Hammer *>(proj) != nullptr);
            if (isTargetProjectile)
            {
                float el, et, er, eb;
                proj->GetBoundingBox(el, et, er, eb);

                // AABB overlap check
                if (myR > el && myL < er && myB > et && myT < eb)
                {
                    proj->Deflect(mario->GetNx());
                    mario->OnParrySuccess(proj);
                    Delete();
                    return;
                }
            }
        }
    }
}

void CounterAttack::Render()
{
    // Invisible helper hitbox entity, nothing to render.
}

void CounterAttack::GetBoundingBox(float &left, float &top, float &right, float &bottom)
{
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}
