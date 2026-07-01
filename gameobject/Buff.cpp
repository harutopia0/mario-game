#include "Buff.h"
#include "../animation/Animations.h"
#include "../gameobject/Block.h"
#include "../physics/Collision.h"

#define BUFF_GRAVITY -0.001f
#define BUFF_WALK_SPEED 0.05f
#define BUFF_SPROUT_SPEED 0.03f

Buff::Buff(float x, float y, int animationId)
    : GameObject(x, y)
{
    this->animationId = animationId;
    this->layer = LAYER_ITEMS;
    this->width = BUFF_WIDTH;
    this->height = BUFF_HEIGHT;

    vx = 0.0f;
    vy = 0.0f;

    isSprouting = false;
    sproutStartY = 0.0f;
    sproutTargetY = 0.0f;

    isStatic = true; // Bắt đầu ở trạng thái tĩnh trước khi được kích hoạt hoặc khi đang sprout
    isDeleted = false;
}

void Buff::StartSprouting(float startY)
{
    isSprouting = true;
    sproutStartY = startY;
    sproutTargetY = startY + 16.0f + 2.0f; // Thêm 2px offset để khi rơi xuống sẽ va chạm chuẩn
    vy = BUFF_SPROUT_SPEED;
    vx = 0.0f;
    isStatic = true; // Trong lúc trồi lên sẽ không check va chạm gạch hay Mario di chuyển ngang
}

void Buff::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void Buff::Update(DWORD dt, vector<GameObject*>* coObjects)
{
    if (isDeleted) return;

    if (isSprouting)
    {
        y += vy * dt;
        if (y >= sproutTargetY)
        {
            y = sproutTargetY;
            vy = 0.0f;
            isSprouting = false;
            isStatic = false; // Bắt đầu chịu tương tác vật lý thông thường
            vx = (rand() % 2 == 0) ? BUFF_WALK_SPEED : -BUFF_WALK_SPEED;
        }
        return;
    }

    // Tác dụng trọng lực
    vy += BUFF_GRAVITY * dt;

    float dx = vx * dt;
    float dy = vy * dt;

    float ml, mt, mr, mb;
    GetBoundingBox(ml, mt, mr, mb);

    // 1. Kiểm tra va chạm trục X với các khối Block
    float min_tx = 1.0f;
    float nx_col = 0;

    if (coObjects != nullptr)
    {
        for (GameObject* obj : *coObjects)
        {
            if (obj == this || obj->IsDeleted()) continue;
            Block* block = dynamic_cast<Block*>(obj);
            if (block && !block->IsOneWay()) // Không va chạm ngang với Platform một chiều
            {
                float sl, st, sr, sb;
                block->GetBoundingBox(sl, st, sr, sb);
                if (mb > st && mt < sb)
                {
                    float t, temp_nx, temp_ny;
                    Collision::GetInstance()->SweptAABB(
                        ml, mt, mr, mb,
                        dx, 0.0f,
                        sl, st, sr, sb,
                        t, temp_nx, temp_ny
                    );
                    if (t < min_tx && temp_nx != 0)
                    {
                        min_tx = t;
                        nx_col = temp_nx;
                    }
                }
            }
        }
    }

    x += min_tx * dx + nx_col * 0.01f;
    if (nx_col != 0)
    {
        vx = -vx; // Đổi chiều khi đập tường
    }

    // Cập nhật lại bounding box sau khi di chuyển ngang
    GetBoundingBox(ml, mt, mr, mb);

    // 2. Kiểm tra va chạm trục Y với các khối Block
    float min_ty = 1.0f;
    float ny_col = 0;

    if (coObjects != nullptr)
    {
        for (GameObject* obj : *coObjects)
        {
            if (obj == this || obj->IsDeleted()) continue;
            Block* block = dynamic_cast<Block*>(obj);
            if (block)
            {
                float sl, st, sr, sb;
                block->GetBoundingBox(sl, st, sr, sb);
                if (mr > sl && ml < sr)
                {
                    float t, temp_nx, temp_ny;
                    Collision::GetInstance()->SweptAABB(
                        ml, mt, mr, mb,
                        0.0f, dy,
                        sl, st, sr, sb,
                        t, temp_nx, temp_ny
                    );
                    if (t < min_ty && temp_ny != 0)
                    {
                        min_ty = t;
                        ny_col = temp_ny;
                    }
                }
            }
        }
    }

    y += min_ty * dy + ny_col * 0.01f;
    if (ny_col != 0)
    {
        vy = 0.0f;
    }

    // Biến mất nếu rơi ra ngoài bản đồ (dưới vực)
    if (y < 0.0f)
    {
        this->Delete();
    }
}

void Buff::Render()
{
    Animation* ani = Animations::GetInstance()->Get(animationId);

    if (ani != NULL)
    {
        ani->Render(x, y, BUFF_WIDTH, BUFF_HEIGHT, 1.0f);
    }
}