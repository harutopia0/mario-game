#include "game/entities/VenusFireTrap.h"
#include "engine/audio/AudioManager.h"
#include "engine/core/Camera.h"
#include "engine/core/Collision.h"
#include "engine/graphics/Animations.h"
#include "game/entities/Mario.h"
#include "game/scenes/GameManager.h"
#include "game/scenes/Map.h"

VenusFireTrap::VenusFireTrap(float x, float y) : Enemy(x, y, 5001)
{
    this->width = 16.0f;
    this->height = 24.0f;
    this->layer = LAYER_BACKGROUND;   // Nằm dưới Pipe, nhưng trên Prop
    this->aimDir = VENUS_DIR_UP_LEFT; // Default

    // Bản thông thường (Bottom-Up)
    this->hiddenY = y - 2.0f - this->height; // Nằm sâu trong ống (đã xác nhận là y - 26)
    this->poppedY = y - 2.0f;                // Nổi lên ngoài ống (đã xác nhận là y - 2)
    this->y = this->poppedY;                 // Bắt đầu ở trạng thái trồi ra ngoài

    aniUpLeft = 5001;
    aniUpRight = 5002;
    aniDownLeft = 5003;
    aniDownRight = 5004;

    SetState(VENUS_STATE_AIMING);
}

void VenusFireTrap::GetBoundingBox(float &left, float &top, float &right, float &bottom)
{
    if (state == VENUS_STATE_HIDING || (y - hiddenY) < 10.0f)
    {
        left = top = right = bottom = 0;
        return;
    }
    left = x;
    top = poppedY;
    right = x + width;
    bottom = y + height;
}

void VenusFireTrap::SetState(int state)
{
    this->state = state;
    switch (state)
    {
    case VENUS_STATE_HIDING:
        vy = 0;
        timerStart = GetTickCount64();
        break;
    case VENUS_STATE_GOING_UP:
        vy = VENUS_SPEED_Y; // Trồi ra: y tiến về poppedY
        break;
    case VENUS_STATE_AIMING:
        vy = 0;
        timerStart = GetTickCount64();
        break;
    case VENUS_STATE_SHOOTING:
        vy = 0;
        timerStart = GetTickCount64();
        break;
    case VENUS_STATE_GOING_DOWN:
        vy = -VENUS_SPEED_Y; // Thụt vào: y tiến về hiddenY
        break;
    }
}

void VenusFireTrap::Update(DWORD dt, vector<GameObject *> *coObjects)
{
    if (isDeleted)
        return;

    if (state == VENUS_STATE_DIE)
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
    if (state != VENUS_STATE_HIDING && (y - hiddenY) >= 10.0f)
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

    Camera *camera = Camera::GetInstance();
    if (camera)
    {
        // Chỉ hoạt động khi thực sự nằm hoàn toàn hoặc một phần trong màn hình
        if (!camera->IsVisible(x, y, width, height))
        {
            timerStart = GetTickCount64(); // Reset timer để khi vào màn hình không bị giật mình
            return;
        }
    }

    Mario *mario = Map::GetInstance()->GetMario();

    bool isSafe = true;
    if (mario)
    {
        float dx = abs(mario->GetX() - this->x);
        if (dx <= VENUS_SAFE_ZONE_WIDTH)
        {
            isSafe = false;
        }
    }

    if (state == VENUS_STATE_HIDING)
    {
        if (isSafe)
        {
            if (GetTickCount64() - timerStart > VENUS_HIDE_DELAY)
            {
                SetState(VENUS_STATE_GOING_UP);
            }
        }
        else
        {
            // Liên tục reset timer nếu Mario vẫn đang đứng gần
            timerStart = GetTickCount64();
        }
    }
    else if (state == VENUS_STATE_GOING_UP)
    {
        y += vy * dt;
        if (y >= poppedY)
        { // Bottom-up trồi ra (y tăng)
            y = poppedY;
            SetState(VENUS_STATE_AIMING);
        }
    }
    else if (state == VENUS_STATE_AIMING)
    {
        if (!isSafe)
        {
            SetState(VENUS_STATE_GOING_DOWN); // Mario lại gần -> thụt xuống
        }
        else
        {
            DetermineAimDirection(mario);
            // Chỉ bắn khi Mario còn sống
            if (mario && !mario->IsDied() && GetTickCount64() - timerStart > VENUS_SHOOT_DELAY)
            {
                ShootFireball(coObjects);
                SetState(VENUS_STATE_SHOOTING);
            }
        }
    }
    else if (state == VENUS_STATE_SHOOTING)
    {
        // Giữ tư thế há miệng 0.75 giây sau khi bắn
        if (GetTickCount64() - timerStart > 750)
        {
            if (!isSafe)
            {
                SetState(VENUS_STATE_GOING_DOWN); // Xoay vòng xong mà Mario lại gần thì thụt xuống
            }
            else
            {
                SetState(VENUS_STATE_AIMING); // Ở xa thì tiếp tục ngắm bắn vòng tiếp theo
            }
        }
    }
    else if (state == VENUS_STATE_GOING_DOWN)
    {
        y += vy * dt;
        if (y <= hiddenY)
        { // Bottom-up thụt vào (y giảm)
            y = hiddenY;
            SetState(VENUS_STATE_HIDING);
        }
    }
}

void VenusFireTrap::DetermineAimDirection(Mario *mario)
{
    if (!mario)
        return;
    bool isLeft = mario->GetX() < this->x;
    // Lưu ý Y tăng lên trên
    bool isAbove = mario->GetY() > this->y + this->height;

    if (isLeft && isAbove)
        aimDir = VENUS_DIR_UP_LEFT;
    else if (isLeft && !isAbove)
        aimDir = VENUS_DIR_DOWN_LEFT;
    else if (!isLeft && isAbove)
        aimDir = VENUS_DIR_UP_RIGHT;
    else
        aimDir = VENUS_DIR_DOWN_RIGHT;

    // Gán animation ID dựa trên aimDir
    if (aimDir == VENUS_DIR_UP_LEFT)
        animationId = aniUpLeft;
    else if (aimDir == VENUS_DIR_DOWN_LEFT)
        animationId = aniDownLeft;
    else if (aimDir == VENUS_DIR_UP_RIGHT)
        animationId = aniUpRight;
    else if (aimDir == VENUS_DIR_DOWN_RIGHT)
        animationId = aniDownRight;
}

void VenusFireTrap::ShootFireball(vector<GameObject *> *coObjects)
{
    float fvx = (aimDir == VENUS_DIR_UP_LEFT || aimDir == VENUS_DIR_DOWN_LEFT) ? -ENEMY_FIREBALL_SPEED : ENEMY_FIREBALL_SPEED;
    float fvy = (aimDir == VENUS_DIR_UP_LEFT || aimDir == VENUS_DIR_UP_RIGHT) ? -ENEMY_FIREBALL_SPEED : ENEMY_FIREBALL_SPEED; // Y increases DOWN, so UP means negative Y

    // Xuất phát đạn từ miệng cây (canh giữa)
    float fx = x + width / 2.0f - ENEMY_FIREBALL_WIDTH / 2.0f;
    float fy = y + height - 8.0f; // Canh đại khái ngay miệng

    Mario *mario = Map::GetInstance()->GetMario();
    if (mario)
    {
        float mx = mario->GetX() + 6.5f; // Giữa Mario
        float my = mario->GetY() + 8.0f;

        float dx = mx - fx;
        float dy = my - fy;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist > 0)
        {
            fvx = (dx / dist) * ENEMY_FIREBALL_SPEED;
            fvy = (dy / dist) * ENEMY_FIREBALL_SPEED;
        }
    }

    EnemyFireball *fb = new EnemyFireball(fx, fy, fvx, fvy);
    Map::GetInstance()->GetObjects().push_back(fb);
    Map::GetInstance()->AddObjectToGrid(fb);
}

void VenusFireTrap::Render()
{
    if (isDeleted)
        return;

    if (state == VENUS_STATE_DIE)
    {
        Animation *ani = Animations::GetInstance()->Get(animationId);
        if (ani)
            ani->Render(x, y, 0, 1);
        return;
    }

    if (died)
        return;

    // Ở trạng thái shooting, ta mượn animation ID mở miệng,
    // tạm thời coi như không cần frame há miệng chuyên biệt nếu gộp chung,
    // hoặc có thể cộng thêm ID nếu muốn há miệng.
    // Thực tế sprite có frame mở miệng và ngậm miệng luân phiên,
    // Animation đã định nghĩa nó nhấp nháy, ta cứ gọi Render.
    Enemy::Render();
}

void VenusFireTrap::OnStomped(Mario *mario)
{
    if (mario != NULL)
    {
        mario->TakeDamage();
    }
    else
    {
        SetState(VENUS_STATE_DIE);
        this->vy = VENUS_SPEED_Y * 5.0f; // Jump up a bit
        this->vx = 0.05f;
        this->died = true;
        this->layer = LAYER_BACKGROUND;
        AudioManager::GetInstance()->PlaySFX("stomp");
        GameManager::GetInstance()->AddScore(200);
        GameManager::GetInstance()->AddKills(1);
    }
}
