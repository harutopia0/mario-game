#include "game/entities/Mario.h"
#include "engine/graphics/Animations.h"
#include "engine/audio/AudioManager.h"
#include "game/objects/Block.h"
#include "game/objects/Breakable.h"
#include "game/items/Buff.h"
#include "game/entities/Enemy.h"
#include "game/entities/Koopa.h"
#include "game/entities/PiranhaPlant.h"
#include "game/entities/VenusFireTrap.h"
#include "game/scenes/Map.h"
#include "engine/core/Camera.h"

#include "game/projectiles/Fireball.h"
#include "game/objects/LuckyBlock.h"
#include "game/objects/Pipe.h"
#include "game/projectiles/RollingBall.h"
#include "game/scenes/GameManager.h"
#include "game/scenes/SceneManager.h"
#include "engine/core/Collision.h"
#include "game/ui/HUD.h"
#include <algorithm>
#include <cmath>

#include "game/entities/MarioInputHandler.h"
#include "game/effects/CounterAttack.h"
#include "game/effects/LightningEffect.h"

Mario::Mario(float x, float y, bool isBig, bool isFire, bool isScissors)
    : GameObject(x, y) {
  isOnGround = false;
  ax = 0.0f;

  this->isBig = isBig;
  this->isFire = isFire;
  this->isScissors = isScissors;

  if (isBig || isFire || isScissors) {
    width = MARIO_BIG_WIDTH;
    height = MARIO_BIG_HEIGHT;
  } else {
    width = MARIO_SMALL_WIDTH;
    height = MARIO_SMALL_HEIGHT;
  }

  if (isFire || isScissors)
    GameManager::GetInstance()->SetLives(3);
  else if (isBig)
    GameManager::GetInstance()->SetLives(2);
  else
    GameManager::GetInstance()->SetLives(1);

  isDead = false;
  deathStart = 0;

  untouchable = false;
  untouchableStart = 0;
  untouchableDuration = 0;
  isCastingSkill = false;
  isStarInvincible = false;
  isPipeAnimating = false;
  lastShootTime = 0;

  isParrying = false;
  parryCooldownTime = 0;

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

  // Rớt khỏi map
  if (y < 0.0f && !isDead) {
    Die();
    vy = 0.45f; // Nảy cao hơn một chút khi rớt vực
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

  // Khi qua màn, Mario tự động đi bộ về bên phải
  if (GameManager::GetInstance()->IsLevelClear() ||
      GameManager::GetInstance()->IsGameWin()) {
    vx = MARIO_WALKING_SPEED;
    nx = 1;
    // Bỏ qua input từ người chơi
  } else if (inputHandler != NULL) {
    inputHandler->KeyState(NULL); // Update continuous keyboard state
  }

  if (isParrying) {
    vx = 0.0f;
    ax = 0.0f;
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
  if (GameManager::GetInstance()->IsLevelClear() ||
      GameManager::GetInstance()->IsGameWin()) {
    pMeterLevel = 0;
    pMeterTimer = 0;
  } else {
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
        if (dynamic_cast<Block *>(e) && !e->IsOneWay()) {
          if (t < min_tx) {
            min_tx = t;
            nx_col = temp_nx;
          }
        } else if (Buff *buff = dynamic_cast<Buff *>(e)) {
          int buffType = buff->GetAnimationId();
          int cardType = 2; // Default to Flower
          if (buffType == 301)
            cardType = 1; // Mushroom
          else if (buffType == 303)
            cardType = 3; // Star
          else if (buffType == 304)
            cardType = 4; // Scissors

          if (GameManager::GetInstance()->AddCard(cardType)) {
            buff->Delete();
            OutputDebugStringA("Buff added to inventory\n");
          }
        } else if (Enemy *enemy = dynamic_cast<Enemy *>(e)) {
          if (!enemy->IsDied() && !enemy->IsFreezed() && isStarInvincible) {
            enemy->SetDied(true);
          }
        }
      }
    }
  }

  x += min_tx * dx + nx_col * 0.01f;
  if (nx_col != 0)
    vx = 0.0f;

  // Khóa mốc x = 0 (vạch xuất phát) để Mario không đi ngược ra khỏi map
  if (x < 0.0f) {
    x = 0.0f;
  }

  // Khi Mario đi tới hoặc vượt qua sát mép phải bản đồ
  float mapEnd = GameManager::GetInstance()->GetMapRightEdge();
  if (mapEnd > 0) {
    if (!GameManager::GetInstance()->IsLevelClear() &&
        !GameManager::GetInstance()->IsGameWin()) {
      if (x > mapEnd - width) {
        int currentLevel = GameManager::GetInstance()->GetLevel();
        if (currentLevel == 5) {
          SceneManager::GetInstance()->ProcessGameWin();
        } else {
          SceneManager::GetInstance()->ProcessLevelClear();
        }
      }
    }
  } else {
    // Fallback if mapEnd is not set
    float rightEdge = Camera::GetInstance()->GetMapWidth();
    if (x > rightEdge - width) {
      if (!GameManager::GetInstance()->IsLevelClear() &&
          !GameManager::GetInstance()->IsGameWin()) {
        int currentLevel = GameManager::GetInstance()->GetLevel();
        if (currentLevel == 5) {
          SceneManager::GetInstance()->ProcessGameWin();
        } else {
          SceneManager::GetInstance()->ProcessLevelClear();
        }
      }
    }
  }

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
        if (dynamic_cast<Block *>(e) && !e->IsOneWay()) {
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

        else if (e->IsOneWay()) {
          if (temp_ny == 1) {
            if (t < min_ty) {
              min_ty = t;
              ny_col = temp_ny;
            }
          }
        } else if (Enemy *enemy = dynamic_cast<Enemy *>(e)) {
          if (!enemy->IsDied() && !enemy->IsFreezed()) {
            if (isStarInvincible) {
              enemy->SetDied(true);
            } else if (temp_ny == 1) {
              if (dynamic_cast<PiranhaPlant *>(enemy) != nullptr ||
                  dynamic_cast<VenusFireTrap *>(enemy) != nullptr) {
                enemy->OnStomped(this);
              } else {
                // Mario dẫm lên đầu enemy bình thường -> nảy lên
                vy = MARIO_JUMP_SPEED_Y * 0.5f;
                OutputDebugStringA("Enemy stomped!\n");
                enemy->OnStomped(this);
              }
            }
            // Lưu ý: va chạm ngang với enemy được xử lý ở phần AABB check bên
            // dưới
          }
        } else if (Buff *buff = dynamic_cast<Buff *>(e)) {
          int buffType = buff->GetAnimationId();
          int cardType = 2; // Default to Flower
          if (buffType == 301)
            cardType = 1; // Mushroom
          else if (buffType == 303)
            cardType = 3; // Star
          else if (buffType == 304)
            cardType = 4; // Scissors

          if (GameManager::GetInstance()->AddCard(cardType)) {
            buff->Delete();
            OutputDebugStringA("Buff added to inventory\n");
          }
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

  // KIỂM TRA VA CHẠM NGANG VỚI ENEMY (AABB overlap - xử lý cả khi mario đứng
  // yên) Thực hiện sau khi đã di chuyển xong để đảm bảo chính xác
  if (!isDead && !isParrying) {
    float fl, ft, fr, fb;
    GetBoundingBox(fl, ft, fr, fb);

    for (UINT i = 0; i < coObjects->size(); i++) {
      GameObject *e = coObjects->at(i);
      if (e == this)
        continue;

      if (Buff *buff = dynamic_cast<Buff *>(e)) {
        if (buff->IsDeleted())
          continue;
        float bl, bt, br, bb;
        buff->GetBoundingBox(bl, bt, br, bb);

        if (!(fr <= bl || fl >= br || fb <= bt || ft >= bb)) {
          int buffType = buff->GetAnimationId();
          int cardType = 2; // Default to Flower
          if (buffType == 301)
            cardType = 1; // Mushroom
          else if (buffType == 303)
            cardType = 3; // Star
          else if (buffType == 304)
            cardType = 4; // Scissors

          if (GameManager::GetInstance()->AddCard(cardType)) {
            buff->Delete();
            OutputDebugStringA("Buff added to inventory (AABB)\n");
          }
        }
        continue;
      }

      Enemy *enemy = dynamic_cast<Enemy *>(e);
      if (!enemy || enemy->IsDied() || enemy->IsFreezed())
        continue;

      float el, et, er, eb;
      enemy->GetBoundingBox(el, et, er, eb);

      // Star Invincible: mở rộng vùng phát hiện để giết cây ăn thịt
      // khi đứng trên ống nước (pipe chặn overlap thật)
      if (isStarInvincible) {
        float margin = 4.0f;
        bool starOverlap = !(fr <= el || fl >= er || 
                             fb + margin <= et || ft - margin >= eb);
        if (starOverlap) {
          enemy->SetDied(true);
          continue;
        }
      }

      // Kiểm tra AABB overlap theo cả 2 trục
      if (fr <= el || fl >= er || fb <= et || ft >= eb)
        continue;

      // MTV (Minimum Translation Vector): axis có overlap nhỏ hơn = hướng va
      // chạm
      float actualOverlapX = min(fr, er) - max(fl, el);
      float actualOverlapY = min(fb, eb) - max(ft, et);



      // Nếu đang untouchable (nhưng không star), bỏ qua damage
      if (untouchable)
        continue;

      Koopa *koopa = dynamic_cast<Koopa *>(enemy);

      if (actualOverlapX >= actualOverlapY) {
        // Va chạm theo chiều dọc (dẫm hoặc đập đầu) → đã được xử lý bởi swept
        // AABB
        continue;
      }

      // Va chạm theo chiều ngang (side hit)
      if (koopa && (koopa->GetState() == KOOPA_STATE_SHELL ||
                    koopa->GetState() == KOOPA_STATE_SHELL_SHAKING)) {
        // Đá mai rùa nằm yên — chỉ kick nếu shell không mới vừa được tạo
        if (!koopa->IsJustShelled()) {
          int dir = ((fl + fr) / 2.0f < (el + er) / 2.0f) ? 1 : -1;
          koopa->Kick(dir);
        }
      } else if (koopa && koopa->GetState() == KOOPA_STATE_SHELL_SPINNING) {
        // Mai rùa đang xoay: chỉ damage nếu HẾT cooldown kick
        // (tránh trường hợp Mario đá xong chạy theo bị dame ngay)
        if (!koopa->IsKickedCooldown()) {
          TakeDamage();
        }
      } else {
        // Đụng enemy bình thường → nhận damage
        TakeDamage();
      }
    }
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

  if (isScissors) {
    if (isParrying) {
      ani = (nx > 0) ? Animations::GetInstance()->Get(710)
                     : Animations::GetInstance()->Get(711);
    } else if (isCastingSkill) {
      ani = (nx > 0) ? Animations::GetInstance()->Get(708)
                     : Animations::GetInstance()->Get(709);
    } else if (!isOnGround) {
      ani = (nx > 0) ? Animations::GetInstance()->Get(704)
                     : Animations::GetInstance()->Get(705);
    } else {
      if (isSkidding)
        ani = (nx > 0) ? Animations::GetInstance()->Get(707)
                       : Animations::GetInstance()->Get(706);
      else if (vx == 0.0f)
        ani = (nx > 0) ? Animations::GetInstance()->Get(700)
                       : Animations::GetInstance()->Get(701);
      else
        ani = (nx > 0) ? Animations::GetInstance()->Get(702)
                       : Animations::GetInstance()->Get(703);
    }
  } else if (isFire) {
    if (isCastingSkill) {
      ani = (nx > 0) ? Animations::GetInstance()->Get(508)
                     : Animations::GetInstance()->Get(509);
    } else if (!isOnGround) {
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
    if (isCastingSkill) {
      ani = (nx > 0) ? Animations::GetInstance()->Get(408)
                     : Animations::GetInstance()->Get(409);
    } else if (!isOnGround) {
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
    isFire = false;
    isScissors = false;
    GameManager::GetInstance()->SetMarioFire(false);
    GameManager::GetInstance()->SetMarioScissors(false);
  } else {
    width = MARIO_SMALL_WIDTH;
    height = MARIO_SMALL_HEIGHT;
    isFire = false;     // Thu nhỏ thì mất luôn lửa
    isScissors = false; // Thu nhỏ thì mất luôn Kéo
    GameManager::GetInstance()->SetMarioFire(false);
    GameManager::GetInstance()->SetMarioScissors(false);
  }

  // Đẩy Mario ra khỏi các block nếu bounding box mới bị overlap
  ResolveOverlap();
}

void Mario::ResolveOverlap() {
  auto &g_objectList = Map::GetInstance()->GetObjects();

  float ml, mt, mr, mb;
  GetBoundingBox(ml, mt, mr, mb);

  for (UINT i = 0; i < g_objectList.size(); i++) {
    GameObject *obj = g_objectList[i];
    if (obj == this || obj->IsDeleted())
      continue;

    Block *block = dynamic_cast<Block *>(obj);
    if (!block || block->IsOneWay())
      continue;

    float bl, bt, br, bb;
    block->GetBoundingBox(bl, bt, br, bb);

    // Kiểm tra AABB overlap
    if (mr > bl && ml < br && mb > bt && mt < bb) {
      // Tính khoảng xâm nhập ở mỗi hướng
      float overlapLeft = mr - bl;
      float overlapRight = br - ml;
      float overlapTop = mb - bt;
      float overlapBottom = bb - mt;

      // Tìm hướng xâm nhập nhỏ nhất để đẩy ra
      float minOverlap = overlapLeft;
      int dir = 0; // 0=left, 1=right, 2=up, 3=down

      if (overlapRight < minOverlap) {
        minOverlap = overlapRight;
        dir = 1;
      }
      if (overlapTop < minOverlap) {
        minOverlap = overlapTop;
        dir = 2;
      }
      if (overlapBottom < minOverlap) {
        minOverlap = overlapBottom;
        dir = 3;
      }

      switch (dir) {
      case 0:
        x -= overlapLeft;
        break; // Đẩy sang trái
      case 1:
        x += overlapRight;
        break; // Đẩy sang phải
      case 2:
        y += overlapTop;
        break; // Đẩy lên trên (hệ tọa độ Y ngược)
      case 3:
        y -= overlapBottom;
        break; // Đẩy xuống dưới
      }

      // Cập nhật lại bounding box sau khi đẩy
      GetBoundingBox(ml, mt, mr, mb);
    }
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
  if (fire) {
    isScissors = false; // Loại trừ lẫn nhau
    isBig = true;
    GameManager::GetInstance()->SetMarioScissors(false);
    GameManager::GetInstance()->SetMarioBig(true);
  }
  GameManager::GetInstance()->SetMarioFire(fire);
}

void Mario::ShootFireball() {
  if (!isFire)
    return;
  if (GetTickCount64() - lastShootTime < 500)
    return; // Cooldown 0.5s

  auto &g_objectList = Map::GetInstance()->GetObjects();

  // Vị trí xuất phát của fireball (dời vào giữa người Mario để tránh lún tường)
  float spawnX = x + width / 2.0f - FIREBALL_WIDTH / 2.0f;
  float spawnY = y + height / 2.0f;

  Fireball *fb = new Fireball(spawnX, spawnY, nx > 0 ? 1 : -1);
  g_objectList.push_back(fb);
  Map::GetInstance()->AddObjectToGrid(fb);

  AudioManager::GetInstance()->PlaySFX("fireball");
  lastShootTime = GetTickCount64();
}

#include "game/projectiles/FireBlast.h"

bool Mario::ShootFireBlast() {
  if (!isFire)
    return false;

  auto &g_objectList = Map::GetInstance()->GetObjects();

  float spawnX = (nx > 0) ? (x + width) : (x - FIREBLAST_WIDTH);
  float spawnY = y + (height / 2.0f) - (FIREBLAST_HEIGHT / 2.0f);

  FireBlast *fb = new FireBlast(spawnX, spawnY, nx > 0 ? 1 : -1);
  g_objectList.push_back(fb);
  Map::GetInstance()->AddObjectToGrid(fb);

  lastShootTime = GetTickCount64();
  return true;
}

bool Mario::ShootRollingBall() {
  if (!isBig)
    return false;

  auto &g_objectList = Map::GetInstance()->GetObjects();

  float spawnX = (nx > 0) ? (x + width) : (x - ROLLINGBALL_WIDTH);
  float spawnY = y + 2.0f;

  float ml = spawnX;
  float mt = spawnY;
  float mr = spawnX + ROLLINGBALL_WIDTH;
  float mb = spawnY + ROLLINGBALL_HEIGHT;

  bool canSpawn = true;
  for (size_t i = 0; i < g_objectList.size(); i++) {
    GameObject *e = g_objectList[i];
    if (e == this || e->IsDeleted())
      continue;

    if (dynamic_cast<Block *>(e) && !e->IsOneWay()) {
      float sl, st, sr, sb;
      e->GetBoundingBox(sl, st, sr, sb);
      if (mr > sl && ml < sr && mb > st && mt < sb) {
        canSpawn = false;
        break;
      }
    }
  }

  if (!canSpawn)
    return false;

  RollingBall *rb = new RollingBall(spawnX, spawnY, nx);
  g_objectList.push_back(rb);
  Map::GetInstance()->AddObjectToGrid(rb);

  lastShootTime = GetTickCount64();
  return true;
}

void Mario::Die() {
  if (isDead || GameManager::GetInstance()->IsGameWin() ||
      GameManager::GetInstance()->IsLevelClear())
    return;

  isDead = true;
  vx = 0;
  vy = 0.2f;
  pMeterLevel = 0; // Reset pMeter
  HUD::GetInstance()->SetPMeter(0);
  deathStart = GetTickCount64();

  SceneManager::GetInstance()->ProcessMarioDeath();
  OutputDebugStringA("Mario died\n");
}

void Mario::TakeDamage() {
  if (untouchable || isDead || GameManager::GetInstance()->IsGameWin() ||
      GameManager::GetInstance()->IsLevelClear())
    return;

  if (isScissors) {
    SetScissors(false);
    GameManager::GetInstance()->SetLives(2);
    untouchable = true;
    untouchableStart = GetTickCount64();
    untouchableDuration = MARIO_UNTOUCHABLE_TIME;
    OutputDebugStringA("Mario lost Scissors -> Big Mario + invincible\n");
  } else if (isFire) {
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
    AudioManager::GetInstance()->PlaySFX("mario_jump");
  }
}

void Mario::SetPressingDown(bool pressing) { this->isPressingDown = pressing; }

// Nhả phím Space giữa chừng khi đang bay lên → cắt vy để nhảy thấp
void Mario::SetHoldingJump(bool holding) {
  if (!holding && vy > MARIO_JUMP_DEFLECT_SPEED) {
    vy = MARIO_JUMP_DEFLECT_SPEED;
  }
}

void Mario::SetScissors(bool scissors) {
  if (scissors && !isScissors) {
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
  isScissors = scissors;
  if (scissors) {
    isFire = false; // Loại trừ lẫn nhau
    isBig = true;
    GameManager::GetInstance()->SetMarioFire(false);
    GameManager::GetInstance()->SetMarioBig(true);
  }
  GameManager::GetInstance()->SetMarioScissors(scissors);
}

bool Mario::StartParry() {
  if (!isScissors)
    return false;
  if (isParrying)
    return false;
  if (GetTickCount64() < parryCooldownTime) {
    AudioManager::GetInstance()->PlaySFX("use-failed");
    return false;
  }

  auto &g_objectList = Map::GetInstance()->GetObjects();

  CounterAttack *ca = new CounterAttack(this);
  g_objectList.push_back(ca);
  Map::GetInstance()->AddObjectToGrid(ca);

  isParrying = true;
  vx = 0.0f;
  ax = 0.0f;

  return true;
}

void Mario::OnParrySuccess(GameObject *enemy) {
  isParrying = false;
  parryCooldownTime = 0; // Reset cooldown immediately

  // Kill the enemy
  Enemy *e = dynamic_cast<Enemy *>(enemy);
  if (e != nullptr && !e->IsDied()) {
    e->OnStomped(NULL); // Die reverse/flipped off-screen
  }

  // Spawn visual effect of red/black lightning at enemy location
  auto &g_objectList = Map::GetInstance()->GetObjects();

  float el, et, er, eb;
  enemy->GetBoundingBox(el, et, er, eb);
  float ecX = el + (er - el) / 2.0f;
  float ecY = et + (eb - et) / 2.0f;

  LightningEffect *le = new LightningEffect(ecX, ecY, this->nx);
  g_objectList.push_back(le);
  Map::GetInstance()->AddObjectToGrid(le);

  // Play sound
  AudioManager::GetInstance()->PlaySFX("parry-sound");
}

void Mario::OnParryFailed() {
  isParrying = false;
  parryCooldownTime = GetTickCount64() + 3000; // 3s cooldown
}
