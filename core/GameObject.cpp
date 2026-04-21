#include "../gameplay/GameObject.h"
#include "../render/Sprites.h"

GameObject::GameObject(float x, float y)
{
    this->x = x;
    this->y = y;
    this->vx = 0.0f;
    this->vy = 0.0f;
    this->nx = 1;
}

void GameObject::Update(DWORD dt, vector<GameObject*>* coObjects)
{
    x += vx * dt;
    y += vy * dt;
}

void GameObject::SweptAABB(
    float ml, float mt, float mr, float mb,
    float dx, float dy, 
    float sl, float st, float sr, float sb, 
    float& t, float& nx, float& ny)
{
    float dx_entry, dx_exit, tx_entry, tx_exit;
    float dy_entry, dy_exit, ty_entry, ty_exit;

    if (dx > 0) {
        dx_entry = sl - mr;
        dx_exit = sr - ml;
    }
    else if (dx < 0) {
        dx_entry = sr - ml;
        dx_exit = sl - mr;
    }
    else {
        dx_entry = -std::numeric_limits<float>::infinity();
        dx_exit = std::numeric_limits<float>::infinity();
    }

    if (dy > 0) {
        dy_entry = st - mb;
        dy_exit = sb - mt;
    }
    else if (dy < 0) {
        dy_entry = sb - mt;
        dy_exit = st - mb;
    }
    else {
        dy_entry = -std::numeric_limits<float>::infinity();
        dy_exit = std::numeric_limits<float>::infinity();
    }

    if (dx == 0) {
        tx_entry = -std::numeric_limits<float>::infinity();
        tx_exit = std::numeric_limits<float>::infinity();
    }
    else {
        tx_entry = dx_entry / dx;
        tx_exit = dx_exit / dx;
    }

    if (dy == 0) {
        ty_entry = -std::numeric_limits<float>::infinity();
        ty_exit = std::numeric_limits<float>::infinity();
    }
    else {
        ty_entry = dy_entry / dy;
        ty_exit = dy_exit / dy;
    }

    float entry_time = max(tx_entry, ty_entry);
        float exit_time = min(tx_exit, ty_exit);

        if (entry_time > exit_time || tx_entry < 0.0f && ty_entry < 0.0f || tx_entry > 1.0f || ty_entry > 1.0f)
        {
            t = 1.0f;
            nx = 0.0f; ny = 0.0f;
            return;
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

void GameObject::RenderBoundingBox()
{
    float l, t, r, b;
    GetBoundingBox(l, t, r, b);

    float width = r - l;
    float height = b - t;

    Sprite* bbox = Sprites::GetInstance()->Get(99999);
    if (bbox != NULL)
    {
        bbox->Draw(l, t, width, height, 0.4f);
    }
}