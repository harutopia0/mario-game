#include "Mario.h"
#include "../audio/AudioManager.h"
#include "../gameplay/SceneManager.h"
#include "../gameobject/Breakable.h"
#include "../gameobject/Brick.h"
#include "../gameobject/Buff.h"
#include "../gameobject/Enemy.h"
#include "../gameobject/Flag.h"
#include "../gameobject/LuckyBlock.h"
#include "../gameobject/Pipe.h"
#include "../animation/Animations.h"
#include "../gameplay/GameManager.h"
#include "../physics/Collision.h"
#include "../ui/HUD.h"
#include <algorithm>
#include <cmath>


#include "../input/MarioInputHandler.h"

Mario::Mario(float x, float y) : GameObject(x, y)
{
	isOnGround = false;
	ax = 0.0f;

	width = MARIO_SMALL_WIDTH;
	height = MARIO_SMALL_HEIGHT;

	lives = 1;
	isBig = false;
	isDead = false;
	deathStart = 0;

	untouchable = false;
	untouchableStart = 0;
	isEnteringPipe = false;

	// Khởi tạo trạng thái P-Meter
	pMeterLevel = 0;
	pMeterTimer = 0;

	isPressingDown = false;
	inputHandler = new MarioInputHandler(this);
}

Mario::~Mario()
{
	if (inputHandler != NULL)
	{
		delete inputHandler;
		inputHandler = NULL;
	}
}

void Mario::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	float realHitboxWidth = 13.0f;
	float offsetX = (this->width - realHitboxWidth) / 2.0f;

	left = x + offsetX;
	top = y;
	right = left + realHitboxWidth;
	bottom = y + this->height;
}

void Mario::Update(DWORD dt, vector<GameObject*>* coObjects)
{
	// Cập nhật số mạng sống lên HUD
	HUD::GetInstance()->SetLives(lives);

	// Thời gian bất tử
	if (untouchable)
	{
		DWORD elapsed = GetTickCount64() - untouchableStart;
		int remain = (MARIO_UNTOUCHABLE_TIME - elapsed) / 1000;
		char debugText[100];
		sprintf_s(debugText, "Mario invincible: %d s\n", remain);
		OutputDebugStringA(debugText);

		if (elapsed > MARIO_UNTOUCHABLE_TIME)
		{
			untouchable = false;
			OutputDebugStringA("Mario vulnerable again\n");
		}
	}

	// Mario chết
	if (isDead)
	{
		vy += MARIO_GRAVITY * dt;
		y += vy * dt;

		if (GetTickCount64() - deathStart > 1500)
		{
			Delete();
			GameManager::GetInstance()->SetGameOver(true);
		}
		return;
	}

	// Xử lý chui ống
	if (isEnteringPipe)
	{
		vy = -0.05f;
		y += vy * dt;

		if (pipeEnterStartY - y > height) {
			x = pipeDestX;
			y = pipeDestY;
			isEnteringPipe = false;
			vy = 0;
		}
		return;
	}

	if (inputHandler != NULL)
	{
		inputHandler->KeyState(NULL); // Update continuous keyboard state
    
  } 
	// KIỂM TRA TRẠNG THÁI KHÓA ĐIỀU KHIỂN KHI QUA MÀN / WIN GAME
	bool isControlLocked = GameManager::GetInstance()->IsLevelClear() || GameManager::GetInstance()->IsGameWin();

	// PHẦN VẬT LÝ DI CHUYỂN (Chỉ nhận phím khi không bị khóa)
	if (!isControlLocked)
	{
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			ax = MARIO_ACCEL_WALK_X;
			nx = 1;
		}
		else if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			ax = -MARIO_ACCEL_WALK_X;
			nx = -1;
		}
		else {
			ax = 0.0f;
		}
	}
	else
	{
		ax = 0.0f; // Không tự gia tốc khi đã thắng, để quán tính và ma sát tự xử lý
	}

	// PHẦN VẬT LÝ DI CHUYỂN
	// CHỈ ÁP DỤNG MA SÁT KHI ĐANG Ở TRÊN MẶT ĐẤT
	if (ax == 0.0f && isOnGround) {
		if (vx > 0) {
			vx -= MARIO_FRICTION * dt;
			if (vx < 0) vx = 0.0f;
		}
		else if (vx < 0) {
			vx += MARIO_FRICTION * dt;
			if (vx > 0) vx = 0.0f;
		}
	}
	vx += ax * dt;

	if (vx > MARIO_WALKING_SPEED) vx = MARIO_WALKING_SPEED;
	if (vx < -MARIO_WALKING_SPEED) vx = -MARIO_WALKING_SPEED;

	// XỬ LÝ PMETER THEO LOGIC VẬT LÝ
	if (std::abs(vx) >= MARIO_WALKING_SPEED * 0.95f)
	{
		if (pMeterLevel < 7)
		{
			pMeterTimer += dt;
			if (pMeterTimer >= PMETER_STEP_UP_TIME)
			{
				pMeterLevel++;
				pMeterTimer = 0;
			}
		}
	}
	else
	{
		if (pMeterLevel > 0)
		{
			pMeterTimer += dt;
			if (pMeterTimer >= PMETER_STEP_DOWN_TIME)
			{
				pMeterLevel--;
				pMeterTimer = 0;
			}
		}
		else {
			pMeterTimer = 0;
		}
	}

	// Đồng bộ mức vận tốc lên HUD
	HUD::GetInstance()->SetPMeter(pMeterLevel);

	// CHỈ CHO PHÉP NHẢY KHI KHÔNG BỊ KHÓA ĐIỀU KHIỂN
	if (!isControlLocked && (GetAsyncKeyState(VK_SPACE) & 0x8000) && isOnGround) {
		vy = MARIO_JUMP_SPEED_Y;
		isOnGround = false;
	}

	vy += MARIO_GRAVITY * dt;

	float dx = vx * dt;
	float dy = vy * dt;

	// QUÉT VA CHẠM TRỤC X (ĐI NGANG)
	float min_tx = 1.0f;
	float nx_col = 0;
	float ml, mt, mr, mb;
	GetBoundingBox(ml, mt, mr, mb);

	for (UINT i = 0; i < coObjects->size(); i++)
	{
		GameObject* e = coObjects->at(i);
		if (e == this) continue;

		float sl, st, sr, sb;
		e->GetBoundingBox(sl, st, sr, sb);

		if (mb > st && mt < sb)
		{
			float t, temp_nx, temp_ny;
			Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, dx, 0.0f, sl, st, sr, sb, t, temp_nx, temp_ny);

			if (t < 1.0f && temp_nx != 0)
			{
				if (dynamic_cast<Brick*>(e) || dynamic_cast<Pipe*>(e) || dynamic_cast<Breakable*>(e) || dynamic_cast<LuckyBlock*>(e)) {
					if (t < min_tx) {
						min_tx = t;
						nx_col = temp_nx;
					}
				}
				else if (Enemy* enemy = dynamic_cast<Enemy*>(e)) {
					if (!enemy->IsDied())
					{
						TakeDamage();
					}
				}
				else if (Buff* buff = dynamic_cast<Buff*>(e)) {
					if (lives < 2)
					{
						lives = 2;
						SetBig(true);
						OutputDebugStringA("Mario became BIG\n");
					}
					buff->Delete();
				}
				// CHẠM CỜ THEO TRỤC X
				else if (Flag* flag = dynamic_cast<Flag*>(e)) {
					int currentLevel = GameManager::GetInstance()->GetLevel();
					if (currentLevel == 3) {
						SceneManager::GetInstance()->ProcessGameWin();
					}
					else {
						SceneManager::GetInstance()->ProcessLevelClear();
					}
					OutputDebugStringA("Win level\n");
				}
			}
		}
	}

	x += min_tx * dx + nx_col * 0.01f;
	if (nx_col != 0) vx = 0.0f;

	// QUÉT VA CHẠM TRỤC Y (RƠI / NHẢY)
	GetBoundingBox(ml, mt, mr, mb);
	float min_ty = 1.0f;
	float ny_col = 0;

	for (UINT i = 0; i < coObjects->size(); i++)
	{
		GameObject* e = coObjects->at(i);
		if (e == this) continue;

		float sl, st, sr, sb;
		e->GetBoundingBox(sl, st, sr, sb);

		if (mr > sl && ml < sr)
		{
			float t, temp_nx, temp_ny;
			Collision::GetInstance()->SweptAABB(ml, mt, mr, mb, 0.0f, dy, sl, st, sr, sb, t, temp_nx, temp_ny);

			if (t < 1.0f && temp_ny != 0)
			{
				if (dynamic_cast<Brick*>(e) || dynamic_cast<Pipe*>(e) || dynamic_cast<Breakable*>(e) || dynamic_cast<LuckyBlock*>(e)) {
					if (t < min_ty) {
						min_ty = t;
						ny_col = temp_ny;
					}

					// XỬ LÝ CHUI ỐNG (Chỉ nhận phím khi không khóa)
					if (Pipe* pipe = dynamic_cast<Pipe*>(e)) {
						if (temp_ny == 1) {
							if (pipe->CanEnter() && isPressingDown) {
								float pipeCenterX = pipe->GetX() + pipe->GetWidth() / 2;
								float marioCenterX = x + width / 2;

								if (abs(pipeCenterX - marioCenterX) < 10.0f) {
									isEnteringPipe = true;
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
					if (Breakable* breakable = dynamic_cast<Breakable*>(e)) {
						if (temp_ny == -1) {
							if (this->IsBig() == true) {
								breakable->Break();
							}
						}
					}

					// XỬ LÝ LUCKY BLOCK
					if (LuckyBlock* lucky = dynamic_cast<LuckyBlock*>(e)) {
						if (temp_ny == -1) {
							lucky->Hit();
						}
					}
				}

				else if (dynamic_cast<Platform*>(e)) {
					if (temp_ny == 1) {
						if (t < min_ty) {
							min_ty = t;
							ny_col = temp_ny;
						}
					}
				}
				else if (Enemy* enemy = dynamic_cast<Enemy*>(e)) {
					if (!enemy->IsDied()) {
						if (temp_ny == 1) {
							vy = MARIO_JUMP_SPEED_Y * 0.5f;
							OutputDebugStringA("Enemy stomped!\n");
							enemy->SetDied(true);
						}
						else if (temp_ny == -1) {
							OutputDebugStringA("Mario damaged by enemy\n");
							TakeDamage();
						}
					}
				}
				else if (Buff* buff = dynamic_cast<Buff*>(e)) {
					if (lives < 2)
					{
						lives = 2;
						SetBig(true);
						OutputDebugStringA("Mario became BIG\n");
					}
					buff->Delete();
				}
				// CHẠM CỜ THEO TRỤC Y
				else if (Flag* flag = dynamic_cast<Flag*>(e)) {
					int currentLevel = GameManager::GetInstance()->GetLevel();
					if (currentLevel == 3) {
						SceneManager::GetInstance()->ProcessGameWin();
					}
					else {
						SceneManager::GetInstance()->ProcessLevelClear();
					}
					OutputDebugStringA("Win level\n");
				}
			}
		}
	}

	y += min_ty * dy + ny_col * 0.01f;

	if (ny_col != 0)
	{
		vy = 0.0f;
		if (ny_col == 1) isOnGround = true;
	}
	else
	{
		isOnGround = false;
	}

	if (IsDied())
	{
		OutputDebugStringA("Game over\n");
	}
}

void Mario::Render()
{
	Animation* ani = NULL;
	bool isSkidding = (vx > 0 && nx < 0) || (vx < 0 && nx > 0);

	if (isDead)
	{
		ani = Animations::GetInstance()->Get(108);
		if (ani != NULL) ani->Render(x, y);
		return;
	}

	if (isBig)
	{
		if (!isOnGround)
		{
			ani = (nx > 0) ? Animations::GetInstance()->Get(404) : Animations::GetInstance()->Get(405);
		}
		else
		{
			if (isSkidding)
				ani = (nx > 0) ? Animations::GetInstance()->Get(407) : Animations::GetInstance()->Get(406);
			else if (vx == 0.0f)
				ani = (nx > 0) ? Animations::GetInstance()->Get(400) : Animations::GetInstance()->Get(401);
			else
				ani = (nx > 0) ? Animations::GetInstance()->Get(402) : Animations::GetInstance()->Get(403);
		}
	}
	else
	{
		if (!isOnGround)
		{
			ani = (nx > 0) ? Animations::GetInstance()->Get(104) : Animations::GetInstance()->Get(105);
		}
		else
		{
			if (isSkidding)
				ani = (nx > 0) ? Animations::GetInstance()->Get(107) : Animations::GetInstance()->Get(106);
			else if (vx == 0.0f)
				ani = (nx > 0) ? Animations::GetInstance()->Get(100) : Animations::GetInstance()->Get(101);
			else
				ani = (nx > 0) ? Animations::GetInstance()->Get(102) : Animations::GetInstance()->Get(103);
		}
	}

	if (untouchable && ((GetTickCount64() / 100) % 2 == 0))
	{
		return;
	}

	if (ani != NULL)
	{
		ani->Render(x, y);
	}
}

void Mario::SetBig(bool big)
{
	if (big && !isBig)
	{
		y -= (MARIO_BIG_HEIGHT - MARIO_SMALL_HEIGHT);
	}
	isBig = big;
	if (big)
	{
		width = MARIO_BIG_WIDTH;
		height = MARIO_BIG_HEIGHT;
	}
	else
	{
		width = MARIO_SMALL_WIDTH;
		height = MARIO_SMALL_HEIGHT;
	}
}

void Mario::Die()
{
	if (isDead || GameManager::GetInstance()->IsGameWin() || GameManager::GetInstance()->IsLevelClear()) return;

	isDead = true;
	vx = 0;
	vy = 0.2f;
	deathStart = GetTickCount64();

	SceneManager::GetInstance()->ProcessMarioDeath();
	OutputDebugStringA("Mario died\n");
}

void Mario::TakeDamage()
{
	if (untouchable || isDead || GameManager::GetInstance()->IsGameWin() || GameManager::GetInstance()->IsLevelClear()) return;

	if (isBig)
	{
		lives = 1;
		SetBig(false);
		untouchable = true;
		untouchableStart = GetTickCount64();
		OutputDebugStringA("Mario shrinked + invincible\n");
	}
	else
	{
		lives = 0;
		Die();
	}
}

void Mario::SetAccelX(float ax)
{
	this->ax = ax;
}

void Mario::SetDirection(int nx)
{
	this->nx = nx;
}

void Mario::Jump()
{
	if (isOnGround)
	{
		vy = MARIO_JUMP_SPEED_Y;
		isOnGround = false;
	}
}

void Mario::SetPressingDown(bool pressing)
{
	this->isPressingDown = pressing;
}

// Nhả phím Space giữa chừng khi đang bay lên → cắt vy để nhảy thấp
void Mario::SetHoldingJump(bool holding)
{
	if (!holding && vy > MARIO_JUMP_DEFLECT_SPEED)
	{
		vy = MARIO_JUMP_DEFLECT_SPEED;
	}
}