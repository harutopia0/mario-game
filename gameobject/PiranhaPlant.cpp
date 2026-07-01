#include "PiranhaPlant.h"
#include "../physics/Collision.h"
#include "Mario.h"
#include "../gameplay/Map.h"
#include "../audio/AudioManager.h"
#include "../animation/Animations.h"

PiranhaPlant::PiranhaPlant(float x, float y) : Enemy(x, y, 5000) {
  this->state = PIRANHA_STATE_HIDING;
  float adjustedY = y - 2.0f;
  this->maxY = adjustedY;                // Nằm sâu nhất
  this->minY = adjustedY - this->height; // Nổi lên hết cỡ
  this->y = this->maxY;                  // Bắt đầu ẩn trong ống
  this->waitTimeStart = GetTickCount64();
  this->layer = LAYER_PROP; // Nằm dưới Pipe
}

void PiranhaPlant::Update(DWORD dt, vector<GameObject *> *coObjects) {
  if (isDeleted)
    return;
  if (isFreezed)
    return;

  if (state == PIRANHA_STATE_DIE) {
    vy += ENEMY_GRAVITY * dt;
    x += vx * dt;
    y += vy * dt;
    if (y < -100.0f) {
      Delete();
    }
    return;
  }

  if (died)
    return;

  y += vy * dt;

  if (state == PIRANHA_STATE_HIDING) {
    if (GetTickCount64() - waitTimeStart > PIRANHA_HIDE_TIME) {
      state = PIRANHA_STATE_GOING_UP;
      vy = -PIRANHA_SPEED; // Trồi lên là giảm Y
    }
  } else if (state == PIRANHA_STATE_GOING_UP) {
    if (y <= minY) {
      y = minY;
      vy = 0;
      state = PIRANHA_STATE_BITING;
      waitTimeStart = GetTickCount64();
    }
  } else if (state == PIRANHA_STATE_BITING) {
    if (GetTickCount64() - waitTimeStart > PIRANHA_BITE_TIME) {
      state = PIRANHA_STATE_GOING_DOWN;
      vy = PIRANHA_SPEED; // Thụt xuống là tăng Y
    }
  } else if (state == PIRANHA_STATE_GOING_DOWN) {
    if (y >= maxY) {
      y = maxY;
      vy = 0;
      state = PIRANHA_STATE_HIDING;
      waitTimeStart = GetTickCount64();
    }
  }
}

void PiranhaPlant::Render() {
  if (isDeleted)
    return;
  if (state == PIRANHA_STATE_DIE) {
      Animation* ani = Animations::GetInstance()->Get(animationId);
      if (ani) ani->Render(x, y, 0, 1); // Flip vertically
      return;
  }
  if (died)
    return;
  Enemy::Render();
}

void PiranhaPlant::GetBoundingBox(float &left, float &top, float &right,
                                  float &bottom) {
  if (state == PIRANHA_STATE_HIDING) {
    left = top = right = bottom = 0;
    return;
  }
  left = x;
  top = y;
  right = x + width;
  bottom = y + height;
}

void PiranhaPlant::OnCollision(GameObject *obj) {
  // Không làm gì, Mario xử lý va chạm với Enemy
}

void PiranhaPlant::OnStomped(Mario *mario) {
  if (mario != NULL) {
    mario->TakeDamage();
  } else {
    this->state = PIRANHA_STATE_DIE;
    this->vy = PIRANHA_SPEED * 5.0f; // Jump a bit
    this->vx = 0.05f;
    this->died = true;
    this->layer = LAYER_BACKGROUND;
    AudioManager::GetInstance()->PlaySFX("stomp");
  }
}
