#include "PiranhaPlant.h"
#include "../physics/Collision.h"
#include "Mario.h"
#include "../gameplay/Map.h"

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
  if (died)
    return;
  if (isFreezed)
    return;

  y += vy * dt;

  if (state == PIRANHA_STATE_HIDING) {
    if (GetTickCount64() - waitTimeStart > PIRANHA_HIDE_TIME) {
      Mario *mario = Map::GetInstance()->GetMario();

      bool isSafe = true;
      if (mario) {
        float dx = abs(mario->GetX() - this->x);
        if (dx <= PIRANHA_SAFE_ZONE_WIDTH) {
          isSafe = false;
        }
      }

      if (isSafe) {
        state = PIRANHA_STATE_GOING_UP;
        vy = -PIRANHA_SPEED; // Trồi lên là giảm Y
      } else {
        // Check again in 500ms
        waitTimeStart = GetTickCount64() - PIRANHA_HIDE_TIME + 500;
      }
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
  if (died)
    return;
  Enemy::Render();
}

void PiranhaPlant::GetBoundingBox(float &left, float &top, float &right,
                                  float &bottom) {
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
    this->died = true; // Chết do bị ném lửa hoặc sao
  }
}
