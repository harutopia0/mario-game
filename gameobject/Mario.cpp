#include "Mario.h"
#include "../animation/Animations.h"
#include "../audio/AudioManager.h"
#include "../gameobject/Breakable.h"
#include "../gameobject/Brick.h"
#include "../gameobject/Buff.h"
#include "../gameobject/Enemy.h"
#include "../gameobject/Flag.h"
#include "../gameobject/LuckyBlock.h"
#include "../gameobject/Pipe.h"
#include "../gameobject/Fireball.h"
#include "../gameobject/RollingBall.h"
#include "../gameplay/GameManager.h"
#include "../gameplay/SceneManager.h"
#include "../physics/Collision.h"
#include "../ui/HUD.h"
#include <algorithm>
#include <cmath>

#include "../input/MarioInputHandler.h"

Mario::Mario(float x, float y, bool isBig, bool isFire) : GameObject(x, y) {
  isOnGround = false;
  ax = 0.0f;

  this->isBig = isBig;
  this->isFire = isFire;

  if (isBig || isFire) {
    width = MARIO_BIG_WIDTH;
    height = MARIO_BIG_HEIGHT;
  } else {
    width = MARIO_SMALL_WIDTH;
    height = MARIO_SMALL_HEIGHT;
  }

  if (isFire) GameManager::GetInstance()->SetLives(3);
  else if (isBig) GameManager::GetInstance()->SetLives(2);
  else GameManager::GetInstance()->SetLives(1);

  isDead = false;
  deathStart = 0;

  untouchable = false;
  untouchableStart = 0;
  untouchableDuration = 0;
  isStarInvincible = false;
  isPipeAnimating = false;
  lastShootTime = 0;

  // Gán layer cho Mario
  layer = LAYER_PLAYER;

  // Khởi tạo trạng thái P-Meter
  pMeterLevel = 0;
  pMeterTimer = 0;

  isPressingDown = false;
  inputHandler = new MarioInputHandler(this);
  layer = LAYER_PLAYER;
}

Mario::~Mario() {
  if (inputHandler != NULL) {
    delete inputHandler;
    inputHandler = NULL;
  }
}

void Mario::GetBoundingBox(float &left, float &top, float &right,
                           float &bottom) {
  float realHitboxWidth = 13.0f;
  float offsetX = (this->width - realHitboxWidth) / 2.0f;

  left = x + offsetX;
  top = y;
  right = left + realHitboxWidth;
  bottom = y + this->height;
}

void Mario::Update(DWORD dt, vector<GameObject *> *coObjects) {

  // Thời gian bất tử
  if (untouchable) {
    DWORD elapsed = GetTickCount64() - untouchableStart;
    int remain = (untouchableDuration - elapsed) / 1000;
    char debugText[100];
    sprintf_s(debugText, "Mario invincible: %d s\n", remain);
    OutputDebugStringA(debugText);

    if (elapsed > untouchableDuration) {
      untouchable = false;
      OutputDebugStringA("Mario vulnerable again\n");
      if (isStarInvincible) {
        isStarInvincible = false;
        AudioManager::GetInstance()->StopEventMusic();
        AudioManager::GetInstance()->ResumeMusic();
      }
    }
  }

  // Mario chết
  if (isDead) {
    vy += MARIO_GRAVITY * dt;
    y += vy * dt;

    if (GetTickCount64() - deathStart > 1500) {
      Delete();
      GameManager::GetInstance()->SetGameOver(true);
    }
    return;
  }

  // Xử lý chui ống
  if (isPipeAnimating) {
    vy = -0.05f;
    y += vy * dt;

    if (pipeEnterStartY - y > height) {
      x = pipeDestX;
      y = pipeDestY;
      isPipeAnimating = false;
      layer = LAYER_PLAYER; // Trả Mario về layer bình thường
      vy = 0;
    }
    return;
  }

  if (inputHandler != NULL) {
    inputHandler->KeyState(NULL); // Update continuous keyboard state
  }

  // PHẦN VẬT LÝ DI CHUYỂN
  // CHỈ ÁP DỤNG MA SÁT KHI ĐANG Ở TRÊN MẶT ĐẤT
  if (ax == 0.0f && isOnGround) {
    if (vx > 0) {
      vx -= MARIO_FRICTION * dt;
      if (vx < 0)
        vx = 0.0f;
    } else if (vx < 0) {
      vx += MARIO_FRICTION * dt;
      if (vx > 0)
        vx = 0.0f;
    }
  }
  vx += ax * dt;

  if (vx > MARIO_WALKING_SPEED)
    vx = MARIO_WALKING_SPEED;
  if (vx < -MARIO_WALKING_SPEED)
    vx = -MARIO_WALKING_SPEED;

  // XỬ LÝ PMETER THEO LOGIC VẬT LÝ
  if (std::abs(vx) >= MARIO_WALKING_SPEED * 0.95f) {
    if (pMeterLevel < 7) {
      pMeterTimer += dt;
      if (pMeterTimer >= PMETER_STEP_UP_TIME) {
        pMeterLevel++;
        pMeterTimer = 0;
      }
    }
  } else {
    if (pMeterLevel > 0) {
      pMeterTimer += dt;
      if (pMeterTimer >= PMETER_STEP_DOWN_TIME) {
        pMeterLevel--;
        pMeterTimer = 0;
      }
    } else {
      pMeterTimer = 0;
    }
  }

  // Đồng bộ mức vận tốc lên HUD
  HUD::GetInstance()->SetPMeter(pMeterLevel);

  vy += MARIO_GRAVITY * dt;

  float dx = vx * dt;
  float dy = vy * dt;

  // QUÉT VA CHẠM TRỤC X (ĐI NGANG)
  float min_tx = 1.0f;
  float nx_col = 0;
  float ml, mt, mr, mb;
  GetBoundingBox(ml, mt, mr, mb);

  for (UINT i = 0; i < coObjects->size(); i++) {
    GameObject *e = coObjects->at(i);
    if (e == this)
      continue;

    float sl, st, sr, sb;
    e->GetBoundingBox(sl, st, sr, sb);

    if (mb > st && mt < sb) {
      float t, temp_nx, temp_ny;
      Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, dx, 0.0f, sl, st, sr,
                                          sb, t, temp_nx, temp_ny);

      if (t < 1.0f && temp_nx != 0) {
        if (dynamic_cast<Brick *>(e) || dynamic_cast<Pipe *>(e) ||
            dynamic_cast<Breakable *>(e) || dynamic_cast<LuckyBlock *>(e)) {
          if (t < min_tx) {
            min_tx = t;
            nx_col = temp_nx;
          }
        } else if (Enemy *enemy = dynamic_cast<Enemy *>(e)) {
          if (!enemy->IsDied()) {
            TakeDamage();
          }
        } else if (Buff *buff = dynamic_cast<Buff *>(e)) {
          int buffType = buff->GetAnimationId();
          int cardType = 2; // Default to Flower
          if (buffType == 301) cardType = 1; // Mushroom
          else if (buffType == 303) cardType = 3; // Star

          if (GameManager::GetInstance()->AddCard(cardType)) {
            buff->Delete();
            OutputDebugStringA("Buff added to inventory\n");
          }
        }
        // CHẠM CỜ THEO TRỤC X
        else if (Flag *flag = dynamic_cast<Flag *>(e)) {
          int currentLevel = GameManager::GetInstance()->GetLevel();
          if (currentLevel == 5) {
            SceneManager::GetInstance()->ProcessGameWin();
          } else {
            SceneManager::GetInstance()->ProcessLevelClear();
          }
          OutputDebugStringA("Win level\n");
        }
      }
    }
  }

  x += min_tx * dx + nx_col * 0.01f;
  if (nx_col != 0)
    vx = 0.0f;

  // QUÉT VA CHẠM TRỤC Y (RƠI / NHẢY)
  GetBoundingBox(ml, mt, mr, mb);
  float min_ty = 1.0f;
  float ny_col = 0;

  for (UINT i = 0; i < coObjects->size(); i++) {
    GameObject *e = coObjects->at(i);
    if (e == this)
      continue;

    float sl, st, sr, sb;
    e->GetBoundingBox(sl, st, sr, sb);

    if (mr > sl && ml < sr) {
      float t, temp_nx, temp_ny;
      Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, 0.0f, dy, sl, st, sr,
                                          sb, t, temp_nx, temp_ny);

      if (t < 1.0f && temp_ny != 0) {
        if (dynamic_cast<Brick *>(e) || dynamic_cast<Pipe *>(e) ||
            dynamic_cast<Breakable *>(e) || dynamic_cast<LuckyBlock *>(e)) {
          if (t < min_ty) {
            min_ty = t;
            ny_col = temp_ny;
          }

          // XỬ LÝ CHUI ỐNG (Chỉ nhận phím khi không khóa)
          if (Pipe *pipe = dynamic_cast<Pipe *>(e)) {
            if (temp_ny == 1) {
              if (pipe->CanEnter() && isPressingDown) {
                float pipeCenterX = pipe->GetX() + pipe->GetWidth() / 2;
                float marioCenterX = x + width / 2;

                if (abs(pipeCenterX - marioCenterX) < 10.0f) {
                  isPipeAnimating = true;
                  layer = LAYER_BACKGROUND; // Chìm ra sau ống
                  pipeDestX = pipe->GetDestX();
                  pipeDestY = pipe->GetDestY();
                  pipeEnterStartY = y;

                  x = pipeCenterX - width / 2;
                  vx = 0;
                }
              }
            }
          }

          // XỬ LÝ PHÁ GẠCH
          if (Breakable *breakable = dynamic_cast<Breakable *>(e)) {
            if (temp_ny == -1) {
              if (this->IsBig() == true) {
                breakable->Break();
              }
            }
          }

          // XỬ LÝ LUCKY BLOCK
          if (LuckyBlock *lucky = dynamic_cast<LuckyBlock *>(e)) {
            if (temp_ny == -1) {
              lucky->Hit();
            }
          }
        }

        else if (dynamic_cast<Platform *>(e)) {
          if (temp_ny == 1) {
            if (t < min_ty) {
              min_ty = t;
              ny_col = temp_ny;
            }
          }
        } else if (Enemy *enemy = dynamic_cast<Enemy *>(e)) {
          if (!enemy->IsDied()) {
            if (temp_ny == 1) {
              vy = MARIO_JUMP_SPEED_Y * 0.5f;
              OutputDebugStringA("Enemy stomped!\n");
              enemy->SetDied(true);
            } else if (temp_ny == -1) {
              OutputDebugStringA("Mario damaged by enemy\n");
              TakeDamage();
            }
          }
        } else if (Buff *buff = dynamic_cast<Buff *>(e)) {
          int buffType = buff->GetAnimationId();
          int cardType = 2; // Default to Flower
          if (buffType == 301) cardType = 1; // Mushroom
          else if (buffType == 303) cardType = 3; // Star

          if (GameManager::GetInstance()->AddCard(cardType)) {
            buff->Delete();
            OutputDebugStringA("Buff added to inventory\n");
          }
        }
        // CHẠM CỜ THEO TRỤC Y
        else if (Flag *flag = dynamic_cast<Flag *>(e)) {
          int currentLevel = GameManager::GetInstance()->GetLevel();
          if (currentLevel == 5) {
            SceneManager::GetInstance()->ProcessGameWin();
          } else {
            SceneManager::GetInstance()->ProcessLevelClear();
          }
          OutputDebugStringA("Win level\n");
        }
      }
    }
  }

  y += min_ty * dy + ny_col * 0.01f;

  if (ny_col != 0) {
    vy = 0.0f;
    if (ny_col == 1)
      isOnGround = true;
  } else {
    isOnGround = false;
  }

  if (IsDied()) {
    OutputDebugStringA("Game over\n");
  }
}

void Mario::Render() {
  Animation *ani = NULL;
  bool isSkidding = (vx > 0 && nx < 0) || (vx < 0 && nx > 0);

  if (isDead) {
    ani = Animations::GetInstance()->Get(108);
    if (ani != NULL)
      ani->Render(x, y);
    return;
  }

  if (isFire) {
    if (!isOnGround) {
      ani = (nx > 0) ? Animations::GetInstance()->Get(504)
                     : Animations::GetInstance()->Get(505);
    } else {
      if (isSkidding)
        ani = (nx > 0) ? Animations::GetInstance()->Get(507)
                       : Animations::GetInstance()->Get(506);
      else if (vx == 0.0f)
        ani = (nx > 0) ? Animations::GetInstance()->Get(500)
                       : Animations::GetInstance()->Get(501);
      else
        ani = (nx > 0) ? Animations::GetInstance()->Get(502)
                       : Animations::GetInstance()->Get(503);
    }
  } else if (isBig) {
    if (!isOnGround) {
      ani = (nx > 0) ? Animations::GetInstance()->Get(404)
                     : Animations::GetInstance()->Get(405);
    } else {
      if (isSkidding)
        ani = (nx > 0) ? Animations::GetInstance()->Get(407)
                       : Animations::GetInstance()->Get(406);
      else if (vx == 0.0f)
        ani = (nx > 0) ? Animations::GetInstance()->Get(400)
                       : Animations::GetInstance()->Get(401);
      else
        ani = (nx > 0) ? Animations::GetInstance()->Get(402)
                       : Animations::GetInstance()->Get(403);
    }
  } else {
    if (!isOnGround) {
      ani = (nx > 0) ? Animations::GetInstance()->Get(104)
                     : Animations::GetInstance()->Get(105);
    } else {
      if (isSkidding)
        ani = (nx > 0) ? Animations::GetInstance()->Get(107)
                       : Animations::GetInstance()->Get(106);
      else if (vx == 0.0f)
        ani = (nx > 0) ? Animations::GetInstance()->Get(100)
                       : Animations::GetInstance()->Get(101);
      else
        ani = (nx > 0) ? Animations::GetInstance()->Get(102)
                       : Animations::GetInstance()->Get(103);
    }
  }

  // Nhấp nháy khi đang biến lớn (xen kẽ trắng/bình thường)
  bool isTransforming = SceneManager::GetInstance()->IsTransforming();
  if (isTransforming) {
    if (ani != NULL) {
      if ((GetTickCount64() / 100) % 2 == 0) {
        ani->Render(x, y, D3DXCOLOR(10.0f, 10.0f, 10.0f, 1.0f)); // Trắng sáng
      } else {
        ani->Render(x, y);
      }
    }
    return;
  }

  // Nhấp nháy đổi màu liên tục khi ăn Sao
  if (isStarInvincible) {
    if (ani != NULL) {
      int cycle = (GetTickCount64() / 50) % 3;
      if (cycle == 0) {
        ani->Render(x, y, D3DXCOLOR(1.2f, 1.2f, 1.2f, 1.0f)); // Trắng hơi sáng
      } else if (cycle == 1) {
        ani->Render(x, y, D3DXCOLOR(1.0f, 0.4f, 0.4f, 1.0f)); // Đỏ sẫm
      } else {
        ani->Render(x, y, D3DXCOLOR(0.5f, 1.0f, 0.5f, 1.0f)); // Xanh lá nhạt
      }
    }
    return;
  }

  // Nhấp nháy khi bất tử do bị thương (ẩn/hiện)
  if (untouchable && !isStarInvincible && ((GetTickCount64() / 100) % 2 == 0)) {
    return;
  }

  if (ani != NULL) {
    ani->Render(x, y);
  }
}

void Mario::SetBig(bool big) {
  if (big && !isBig) {
    
    // Bật trạng thái chớp (tàng hình) và phát âm thanh
    untouchable = true;
    untouchableStart = GetTickCount64();
    untouchableDuration = 1000; // 1 giây chớp cho nấm
    AudioManager::GetInstance()->PlaySFX("power_up");

    // Tạm dừng game 1 giây khi biến lớn
    SceneManager::GetInstance()->ProcessTransform();
  }
  isBig = big;
  GameManager::GetInstance()->SetMarioBig(big);
  if (big) {
    width = MARIO_BIG_WIDTH;
    height = MARIO_BIG_HEIGHT;
  } else {
    width = MARIO_SMALL_WIDTH;
    height = MARIO_SMALL_HEIGHT;
    isFire = false; // Thu nhỏ thì mất luôn lửa
    GameManager::GetInstance()->SetMarioFire(false);
  }
}

void Mario::SetFire(bool fire) {
  if (fire && !isFire) {
    if (!isBig) {
      isBig = true;
      width = MARIO_BIG_WIDTH;
      height = MARIO_BIG_HEIGHT;
    }
    
    // Bật trạng thái chớp (tàng hình) và phát âm thanh
    untouchable = true;
    untouchableStart = GetTickCount64();
    untouchableDuration = 1000;
    AudioManager::GetInstance()->PlaySFX("power_up");

    // Tạm dừng game 1 giây khi biến hóa
    SceneManager::GetInstance()->ProcessTransform();
  }
  isFire = fire;
  GameManager::GetInstance()->SetMarioFire(fire);
}

void Mario::ShootFireball() {
  if (!isFire) return;
  if (GetTickCount64() - lastShootTime < 500) return; // Cooldown 0.5s

  extern std::vector<GameObject*> g_objectList;
  extern void AddObjectToGrid(GameObject* obj);

  // Vị trí xuất phát của fireball (dời vào giữa người Mario để tránh lún tường)
  float spawnX = x + width / 2.0f - FIREBALL_WIDTH / 2.0f;
  float spawnY = y + height / 2.0f;

  Fireball* fb = new Fireball(spawnX, spawnY, nx > 0 ? 1 : -1);
  g_objectList.push_back(fb);
  AddObjectToGrid(fb);

  AudioManager::GetInstance()->PlaySFX("fireball");
  lastShootTime = GetTickCount64();
}

#include "FireBlast.h"

void Mario::ShootFireBlast() {
  if (!isFire) return;

  extern std::vector<GameObject*> g_objectList;
  extern void AddObjectToGrid(GameObject* obj);

  float spawnX = (nx > 0) ? (x + width) : (x - FIREBLAST_WIDTH);
  float spawnY = y + (height / 2.0f) - (FIREBLAST_HEIGHT / 2.0f);

  FireBlast* blast = new FireBlast(spawnX, spawnY, nx);
  g_objectList.push_back(blast);
  AddObjectToGrid(blast);

  AudioManager::GetInstance()->PlaySFX("fireball"); // Có thể đổi sound effect khác nếu có
  lastShootTime = GetTickCount64();
}

void Mario::ShootRollingBall() {
  extern std::vector<GameObject*> g_objectList;
  extern void AddObjectToGrid(GameObject* obj);

  float spawnX = (nx > 0) ? (x + width) : (x - ROLLINGBALL_WIDTH);
  float spawnY = y + (height / 2.0f) - (ROLLINGBALL_HEIGHT / 2.0f);

  RollingBall* rb = new RollingBall(spawnX, spawnY, nx);
  g_objectList.push_back(rb);
  AddObjectToGrid(rb);

  AudioManager::GetInstance()->PlaySFX("fireball");
  lastShootTime = GetTickCount64();
}

void Mario::Die() {
  if (isDead || GameManager::GetInstance()->IsGameWin() ||
      GameManager::GetInstance()->IsLevelClear())
    return;

  isDead = true;
  vx = 0;
  vy = 0.2f;
  deathStart = GetTickCount64();

  SceneManager::GetInstance()->ProcessMarioDeath();
  OutputDebugStringA("Mario died\n");
}

void Mario::TakeDamage() {
  if (untouchable || isDead || GameManager::GetInstance()->IsGameWin() ||
      GameManager::GetInstance()->IsLevelClear())
    return;

  if (isFire) {
    SetFire(false);
    GameManager::GetInstance()->SetLives(2);
    untouchable = true;
    untouchableStart = GetTickCount64();
    untouchableDuration = MARIO_UNTOUCHABLE_TIME;
    OutputDebugStringA("Mario lost fire -> Big Mario + invincible\n");
  } else if (isBig) {
    GameManager::GetInstance()->SetLives(1);
    SetBig(false);
    untouchable = true;
    untouchableStart = GetTickCount64();
    untouchableDuration = MARIO_UNTOUCHABLE_TIME; // 5 giây khi bị thương
    OutputDebugStringA("Mario shrinked + invincible\n");
  } else {
    GameManager::GetInstance()->SetLives(0);
    Die();
  }
}

void Mario::SetAccelX(float ax) { this->ax = ax; }

void Mario::SetDirection(int nx) { this->nx = nx; }

void Mario::Jump() {
  if (isOnGround) {
    vy = MARIO_JUMP_SPEED_Y;
    isOnGround = false;
  }
}

void Mario::SetPressingDown(bool pressing) { this->isPressingDown = pressing; }

// Nhả phím Space giữa chừng khi đang bay lên → cắt vy để nhảy thấp
void Mario::SetHoldingJump(bool holding) {
  if (!holding && vy > MARIO_JUMP_DEFLECT_SPEED) {
    vy = MARIO_JUMP_DEFLECT_SPEED;
  }
}