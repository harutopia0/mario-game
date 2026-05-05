#include "Collision.h"
#include <limits>

Collision* Collision::__instance = nullptr;

Collision* Collision::GetInstance()
{
    if (__instance == nullptr) __instance = new Collision();
    return __instance;
}

void Collision::SweptAABB(
    float ml, float mt, float mr, float mb,
    float dx, float dy,
    float sl, float st, float sr, float sb,
    float& t, float& nx, float& ny)
{
    float dx_entry, dx_exit, tx_entry, tx_exit;
    float dy_entry, dy_exit, ty_entry, ty_exit;

    if (dx > 0) { dx_entry = sl - mr; dx_exit = sr - ml; }
    else if (dx < 0) { dx_entry = sr - ml; dx_exit = sl - mr; }
    else { dx_entry = -std::numeric_limits<float>::infinity(); dx_exit = std::numeric_limits<float>::infinity(); }

    if (dy > 0) { dy_entry = st - mb; dy_exit = sb - mt; }
    else if (dy < 0) { dy_entry = sb - mt; dy_exit = st - mb; }
    else { dy_entry = -std::numeric_limits<float>::infinity(); dy_exit = std::numeric_limits<float>::infinity(); }

    if (dx == 0) { tx_entry = -std::numeric_limits<float>::infinity(); tx_exit = std::numeric_limits<float>::infinity(); }
    else { tx_entry = dx_entry / dx; tx_exit = dx_exit / dx; }

    if (dy == 0) { ty_entry = -std::numeric_limits<float>::infinity(); ty_exit = std::numeric_limits<float>::infinity(); }
    else { ty_entry = dy_entry / dy; ty_exit = dy_exit / dy; }

    float entry_time = max(tx_entry, ty_entry);
    float exit_time = min(tx_exit, ty_exit);

    if (entry_time > exit_time || tx_entry < 0.0f && ty_entry < 0.0f || tx_entry > 1.0f || ty_entry > 1.0f) {
        t = 1.0f; nx = 0.0f; ny = 0.0f; return;
    }

    t = entry_time;
    if (tx_entry > ty_entry) {
        if (dx_entry < 0.0f) { nx = 1.0f; ny = 0.0f; }
        else { nx = -1.0f; ny = 0.0f; }
    }
    else {
        if (dy_entry < 0.0f) { nx = 0.0f; ny = 1.0f; }
        else { nx = 0.0f; ny = -1.0f; }
    }
}

void Collision::Scan(GameObject* objSrc, float dx, float dy, vector<GameObject*>* coObjects, vector<LPCOLLISIONEVENT>& coEvents)
{
    float ml, mt, mr, mb;
    objSrc->GetBoundingBox(ml, mt, mr, mb);

    for (UINT i = 0; i < coObjects->size(); i++)
    {
        GameObject* e = coObjects->at(i);
        if (e == objSrc) continue;

        float sl, st, sr, sb;
        e->GetBoundingBox(sl, st, sr, sb);

        float t, nx, ny;
        SweptAABB(ml, mt, mr, mb, dx, dy, sl, st, sr, sb, t, nx, ny);

        if (t < 1.0f && (nx != 0 || ny != 0))
        {
            coEvents.push_back(new CollisionEvent(t, nx, ny, e));
        }
    }
}