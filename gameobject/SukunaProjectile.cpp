#include "SukunaProjectile.h"
#include "../animation/Animations.h"
#include "../physics/Collision.h"
#include "Enemy.h"
#include "SukunaSlash.h"

SukunaProjectile::SukunaProjectile(float x, float y, int direction) : Projectile(x, y, direction) {
    width = 24.0f;
    height = 24.0f;
    animationId = 805; // Sukuna projectile animation
    vx = direction * 0.3f;
    vy = 0.0f;
    creationTime = GetTickCount64();
}

void SukunaProjectile::Update(DWORD dt, vector<GameObject*>* coObjects) {
    if (isDeleted) return;

    // Tự hủy sau 3 giây nếu không chạm quái
    if (GetTickCount64() - creationTime > 3000) {
        this->Delete();
        return;
    }

    float dx = vx * dt;
    float dy = vy * dt;

    float ml, mt, mr, mb;
    GetBoundingBox(ml, mt, mr, mb);

    extern std::vector<GameObject*> g_objectList;
    extern void AddObjectToGrid(GameObject* obj);

    for (UINT i = 0; i < coObjects->size(); i++) {
        GameObject* e = coObjects->at(i);
        if (e == this || e->IsDeleted()) continue;

        Enemy* enemy = dynamic_cast<Enemy*>(e);
        if (enemy && !enemy->IsDied() && !enemy->IsFreezed()) {
            float sl, st, sr, sb;
            enemy->GetBoundingBox(sl, st, sr, sb);

            // Chỉ tính va chạm nếu có trùng khớp trên trục Y (giống đạn lửa Fireball)
            if (mb > st && mt < sb) {
                // Kiểm tra va chạm swept AABB
                float t, temp_nx, temp_ny;
                Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, dx, dy, sl, st, sr, sb, t, temp_nx, temp_ny);

                // Kiểm tra va chạm overlap trực tiếp phòng trường hợp spawn đè lên nhau
                bool isOverlap = (mr > sl && ml < sr);

                if ((t < 1.0f) || isOverlap) {
                    // Đóng băng quái vật
                    enemy->SetFreezed(true);

                    // Lấy trung tâm quái vật để vẽ hiệu ứng chém
                    float enemyCenterX = sl + (sr - sl) / 2.0f;
                    float enemyCenterY = st + (sb - st) / 2.0f;

                    // Căn giữa hiệu ứng 80x80 lên quái
                    float slashX = enemyCenterX - 40.0f;
                    float slashY = enemyCenterY - 40.0f;

                    SukunaSlash* slash = new SukunaSlash(slashX, slashY, enemy);
                    g_objectList.push_back(slash);
                    AddObjectToGrid(slash);

                    this->Delete();
                    return;
                }
            }
        }
    }

    x += dx;
    y += dy;
}

void SukunaProjectile::Render() {
    Animation* ani = Animations::GetInstance()->Get(animationId);
    if (ani != NULL) {
        float drawX = x + width / 2.0f - 40.0f; // 40 is half of sprite width (80)
        float drawY = y + height / 2.0f - 40.0f; // 40 is half of sprite height (80)
        ani->Render(drawX, drawY);
    }
}
