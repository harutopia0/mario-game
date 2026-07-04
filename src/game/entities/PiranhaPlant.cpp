#include "game/entities/PiranhaPlant.h"
#include "engine/audio/AudioManager.h"
#include "engine/core/Collision.h"
#include "engine/graphics/Animations.h"
#include "game/entities/Mario.h"
#include "game/scenes/GameManager.h"
#include "game/scenes/Map.h"

PiranhaPlant::PiranhaPlant(float x, float y) : Enemy(x, y, 5000)
{
    this->state = PIRANHA_STATE_HIDING;
    float adjustedY = y - 2.0f;
    this->hiddenY = adjustedY - this->height; // Nằm sâu nhất
    this->poppedY = adjustedY;                // Nổi lên hết cỡ
    this->y = this->hiddenY;                  // Bắt đầu ẩn trong ống
    this->waitTimeStart = GetTickCount64();
    this->layer = LAYER_BACKGROUND; // Nằm dưới Pipe, nhưng trên Prop
}

void PiranhaPlant::Update(DWORD dt, vector<GameObject *> *coObjects)
{
    if (isDeleted)
        return;
    if (isFreezed)
        return;

    if (state == PIRANHA_STATE_DIE)
    {
        vy += ENEMY_GRAVITY * dt;
        x += vx * dt;
        y += vy * dt;
        if (y < -100.0f)
        {
            Delete();
        }
        return;
    }

    if (died)
        return;

    // Khi cây đang hiện (không hiding) và đã trồi lên đủ cao, kiểm tra va chạm với Mario có Sao
    if (state != PIRANHA_STATE_HIDING && (y - hiddenY) >= 10.0f)
    {
        Mario *mario = Map::GetInstance()->GetMario();
        if (mario && mario->isStarInvincible && !mario->IsDied())
        {
            float ml, mt, mr, mb;
            mario->GetBoundingBox(ml, mt, mr, mb);
            float pl = x, pt = y, pr = x + width, pb = y + height;
            if (!(mr <= pl || ml >= pr || mb <= pt || mt >= pb))
            {
                SetDied(true);
                return;
            }
        }
    }

    y += vy * dt;

    if (state == PIRANHA_STATE_HIDING)
    {
        if (GetTickCount64() - waitTimeStart > PIRANHA_HIDE_TIME)
        {
            state = PIRANHA_STATE_GOING_UP;
            vy = PIRANHA_SPEED; // Trồi lên là tăng Y
        }
    }
    else if (state == PIRANHA_STATE_GOING_UP)
    {
        if (y >= poppedY)
        {
            y = poppedY;
            vy = 0;
            state = PIRANHA_STATE_BITING;
            waitTimeStart = GetTickCount64();
        }
    }
    else if (state == PIRANHA_STATE_BITING)
    {
        if (GetTickCount64() - waitTimeStart > PIRANHA_BITE_TIME)
        {
            state = PIRANHA_STATE_GOING_DOWN;
            vy = -PIRANHA_SPEED; // Thụt xuống là giảm Y
        }
    }
    else if (state == PIRANHA_STATE_GOING_DOWN)
    {
        if (y <= hiddenY)
        {
            y = hiddenY;
            vy = 0;
            state = PIRANHA_STATE_HIDING;
            waitTimeStart = GetTickCount64();
        }
    }
}

void PiranhaPlant::Render()
{
    if (isDeleted)
        return;
    if (state == PIRANHA_STATE_DIE)
    {
        Animation *ani = Animations::GetInstance()->Get(animationId);
        if (ani)
            ani->Render(x, y, 0, 1); // Flip vertically
        return;
    }
    if (died)
        return;
    Enemy::Render();
}

void PiranhaPlant::GetBoundingBox(float &left, float &top, float &right,
                                  float &bottom)
{
    if (state == PIRANHA_STATE_HIDING || (y - hiddenY) < 10.0f)
    {
        left = top = right = bottom = 0;
        return;
    }
    left = x;
    top = poppedY;
    right = x + width;
    bottom = y + height;
}

void PiranhaPlant::OnCollision(GameObject *obj)
{
    // Không làm gì, Mario xử lý va chạm với Enemy
}

void PiranhaPlant::OnStomped(Mario *mario)
{
    if (mario != NULL)
    {
        mario->TakeDamage();
    }
    else
    {
        this->state = PIRANHA_STATE_DIE;
        this->vy = PIRANHA_SPEED * 5.0f; // Bay lên (y tăng = lên trên)
        this->vx = 0.05f;
        this->died = true;
        this->layer = LAYER_ENEMIES; // Render trước pipe để thấy animation chết
        AudioManager::GetInstance()->PlaySFX("stomp");
        GameManager::GetInstance()->AddScore(200);
        GameManager::GetInstance()->AddKills(1);
    }
}
