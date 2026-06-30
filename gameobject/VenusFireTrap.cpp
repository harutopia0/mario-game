#include "VenusFireTrap.h"
#include "../gameobject/Mario.h"
#include "../gameplay/Map.h"
#include "../physics/Collision.h"
#include "../render/Camera.h"

VenusFireTrap::VenusFireTrap(float x, float y, bool isUpsideDown) : Enemy(x, y, 5001) {
    this->isUpsideDown = isUpsideDown;
    this->width = 16.0f;
    this->height = 24.0f;
    this->layer = LAYER_PROP; // Nằm dưới Pipe
    this->aimDir = VENUS_DIR_UP_LEFT; // Default

    if (isUpsideDown) {
        // Bản treo ngược (Top-Down)
        this->hiddenY = y + this->height; // Giấu hẳn vào trong ống
        this->poppedY = y - 2.0f; // Thò xuống hết cỡ
        this->y = this->poppedY; // Bắt đầu ở trạng thái trồi ra
        
        aniUpLeft = 5011;
        aniUpRight = 5012;
        aniDownLeft = 5013;
        aniDownRight = 5014;
    } else {
        // Bản thông thường (Bottom-Up)
        this->hiddenY = y - 2.0f - this->height; // Nằm sâu trong ống (đã xác nhận là y - 26)
        this->poppedY = y - 2.0f; // Nổi lên ngoài ống (đã xác nhận là y - 2)
        this->y = this->poppedY; // Bắt đầu ở trạng thái trồi ra ngoài
        
        aniUpLeft = 5001;
        aniUpRight = 5002;
        aniDownLeft = 5003;
        aniDownRight = 5004;
    }

    SetState(VENUS_STATE_AIMING);
}

void VenusFireTrap::GetBoundingBox(float &left, float &top, float &right, float &bottom) {
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void VenusFireTrap::SetState(int state) {
    this->state = state;
    switch (state) {
        case VENUS_STATE_HIDING:
            vy = 0;
            timerStart = GetTickCount64();
            break;
        case VENUS_STATE_GOING_UP:
            vy = isUpsideDown ? -VENUS_SPEED_Y : VENUS_SPEED_Y; // Trồi ra: y tiến về poppedY
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
            vy = isUpsideDown ? VENUS_SPEED_Y : -VENUS_SPEED_Y; // Thụt vào: y tiến về hiddenY
            break;
    }
}

void VenusFireTrap::Update(DWORD dt, vector<GameObject*>* coObjects) {
    if (died) return;

    Camera* camera = Camera::GetInstance();
    if (camera) {
        // Mở rộng viền một chút (ví dụ 16px) để không bị ngắt cái rụp khi vừa nhích khỏi mép
        if (!camera->IsVisible(x - 16.0f, y - 16.0f, width + 32.0f, height + 32.0f)) {
            timerStart = GetTickCount64(); // Reset timer để khi vào màn hình không bị giật mình
            return;
        }
    }
    
    Mario *mario = Map::GetInstance()->GetMario();

    bool isSafe = true;
    if (mario) {
        float dx = abs(mario->GetX() - this->x);
        if (dx <= VENUS_SAFE_ZONE_WIDTH) {
            isSafe = false;
        }
    }

    if (state == VENUS_STATE_HIDING) {
        if (isSafe) {
            if (GetTickCount64() - timerStart > VENUS_HIDE_DELAY) {
                SetState(VENUS_STATE_GOING_UP);
            }
        } else {
            // Liên tục reset timer nếu Mario vẫn đang đứng gần
            timerStart = GetTickCount64();
        }
    } 
    else if (state == VENUS_STATE_GOING_UP) {
        y += vy * dt;
        if (!isUpsideDown && y >= poppedY) { // Bottom-up trồi ra (y tăng)
            y = poppedY;
            SetState(VENUS_STATE_AIMING);
        } else if (isUpsideDown && y <= poppedY) { // Top-down trồi ra (y giảm)
            y = poppedY;
            SetState(VENUS_STATE_AIMING);
        }
    } 
    else if (state == VENUS_STATE_AIMING) {
        if (!isSafe) {
            SetState(VENUS_STATE_GOING_DOWN); // Mario lại gần -> thụt xuống
        } else {
            DetermineAimDirection(mario);
            // Chỉ bắn khi Mario còn sống
            if (mario && !mario->IsDied() && GetTickCount64() - timerStart > VENUS_SHOOT_DELAY) {
                ShootFireball(coObjects);
                SetState(VENUS_STATE_SHOOTING);
            }
        }
    } 
    else if (state == VENUS_STATE_SHOOTING) {
        // Giữ tư thế há miệng 0.5 giây sau khi bắn
        if (GetTickCount64() - timerStart > 500) {
            if (!isSafe) {
                SetState(VENUS_STATE_GOING_DOWN); // Xoay vòng xong mà Mario lại gần thì thụt xuống
            } else {
                SetState(VENUS_STATE_AIMING); // Ở xa thì tiếp tục ngắm bắn vòng tiếp theo
            }
        }
    } 
    else if (state == VENUS_STATE_GOING_DOWN) {
        y += vy * dt;
        if (!isUpsideDown && y <= hiddenY) { // Bottom-up thụt vào (y giảm)
            y = hiddenY;
            SetState(VENUS_STATE_HIDING);
        } else if (isUpsideDown && y >= hiddenY) { // Top-down thụt vào (y tăng)
            y = hiddenY;
            SetState(VENUS_STATE_HIDING);
        }
    }
}

void VenusFireTrap::DetermineAimDirection(Mario* mario) {
    if (!mario) return;
    bool isLeft = mario->GetX() < this->x;
    // Lưu ý Y tăng lên trên
    bool isAbove = mario->GetY() > this->y + this->height;
    
    if (isLeft && isAbove) aimDir = VENUS_DIR_UP_LEFT;
    else if (isLeft && !isAbove) aimDir = VENUS_DIR_DOWN_LEFT;
    else if (!isLeft && isAbove) aimDir = VENUS_DIR_UP_RIGHT;
    else aimDir = VENUS_DIR_DOWN_RIGHT;
    
    // Gán animation ID dựa trên aimDir
    if (aimDir == VENUS_DIR_UP_LEFT) animationId = aniUpLeft;
    else if (aimDir == VENUS_DIR_DOWN_LEFT) animationId = aniDownLeft;
    else if (aimDir == VENUS_DIR_UP_RIGHT) animationId = aniUpRight;
    else if (aimDir == VENUS_DIR_DOWN_RIGHT) animationId = aniDownRight;
}

void VenusFireTrap::ShootFireball(vector<GameObject*>* coObjects) {
    float fvx = (aimDir == VENUS_DIR_UP_LEFT || aimDir == VENUS_DIR_DOWN_LEFT) ? -ENEMY_FIREBALL_SPEED : ENEMY_FIREBALL_SPEED;
    float fvy = (aimDir == VENUS_DIR_UP_LEFT || aimDir == VENUS_DIR_UP_RIGHT) ? -ENEMY_FIREBALL_SPEED : ENEMY_FIREBALL_SPEED; // Y increases DOWN, so UP means negative Y
    
    // Xuất phát đạn từ miệng cây (canh giữa)
    float fx = x + width / 2.0f - ENEMY_FIREBALL_WIDTH / 2.0f;
    float fy = y + height - 8.0f; // Canh đại khái ngay miệng
    if (isUpsideDown) fy = y + 8.0f;
    
    Mario *mario = Map::GetInstance()->GetMario();
    if (mario) {
        float mx = mario->GetX() + 6.5f; // Giữa Mario
        float my = mario->GetY() + 8.0f;
        
        float dx = mx - fx;
        float dy = my - fy;
        float dist = sqrt(dx * dx + dy * dy);
        
        if (dist > 0) {
            fvx = (dx / dist) * ENEMY_FIREBALL_SPEED;
            fvy = (dy / dist) * ENEMY_FIREBALL_SPEED;
        }
    }
    
    EnemyFireball* fb = new EnemyFireball(fx, fy, fvx, fvy);
    Map::GetInstance()->GetObjects().push_back(fb);
    Map::GetInstance()->AddObjectToGrid(fb);
}

void VenusFireTrap::Render() {
    if (died) return;
    
    // Ở trạng thái shooting, ta mượn animation ID mở miệng, 
    // tạm thời coi như không cần frame há miệng chuyên biệt nếu gộp chung,
    // hoặc có thể cộng thêm ID nếu muốn há miệng.
    // Thực tế sprite có frame mở miệng và ngậm miệng luân phiên, 
    // Animation đã định nghĩa nó nhấp nháy, ta cứ gọi Render.
    Enemy::Render();
}

void VenusFireTrap::OnStomped(Mario* mario) {
    if (mario != NULL) {
        mario->TakeDamage();
    } else {
        this->died = true;
    }
}
