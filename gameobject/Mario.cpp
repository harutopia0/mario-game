#include "Mario.h"
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
#include <algorithm>

#define MARIO_JUMP_SPEED_Y      0.27f 
#define MARIO_GRAVITY           -0.00067f
#define MARIO_WALKING_SPEED		0.15f
#define MARIO_ACCEL_WALK_X		0.0005f 
#define MARIO_FRICTION			0.0004f
// Thời gian bất tử
#define MARIO_UNTOUCHABLE_TIME 5000

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
	//Thời gian bất tử`
	if (untouchable)
	{
		DWORD elapsed =
			GetTickCount64() - untouchableStart;
		int remain = (MARIO_UNTOUCHABLE_TIME - elapsed) / 1000;
		char debugText[100];
		sprintf_s(debugText, "Mario invincible: %d s\n",remain);

		OutputDebugStringA(debugText);

		if (elapsed > MARIO_UNTOUCHABLE_TIME)
		{
			untouchable = false;
			OutputDebugStringA("Mario vulnerable again\n");
		}
	}
	//Mario chết
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

	// PHẦN VẬT LÝ DI CHUYỂN
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

	if (ax == 0.0f) {
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

	if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && isOnGround) {
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
				// 1. ĐỤNG BLOCK (Cản đường lại)
				if (dynamic_cast<Brick*>(e) || dynamic_cast<Pipe*>(e) || dynamic_cast<Breakable*>(e) || dynamic_cast<LuckyBlock*>(e)) {
					if (t < min_tx) {
						min_tx = t;
						nx_col = temp_nx;
					}
				}
				// 2. ĐỤNG QUÁI VẬT (Bị thương)
				else if (Enemy* enemy = dynamic_cast<Enemy*>(e)) {
					if (!enemy->IsDied())
					{
						TakeDamage();
					}

				}
				// 3. ĐỤNG BUFF
				else if (Buff* buff = dynamic_cast<Buff*>(e)) {
					if (lives < 2)
					{
						lives = 2;
						SetBig(true);

						OutputDebugStringA("Mario became BIG\n");
					}

					buff->Delete();
				}
				// 4. CHẠM CỜ
				else if (Flag* flag = dynamic_cast<Flag*>(e)) {
					GameManager::GetInstance()->SetGameWin(true);
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
				// 1. ĐỤNG BLOCK (Cản cả trên lẫn dưới)
				if (dynamic_cast<Brick*>(e) || dynamic_cast<Pipe*>(e) || dynamic_cast<Breakable*>(e) || dynamic_cast<LuckyBlock*>(e)) {
					if (t < min_ty) {
						min_ty = t;
						ny_col = temp_ny;
					}

					// XỬ LÝ CHUI ỐNG
					if (Pipe* pipe = dynamic_cast<Pipe*>(e)) {
						if (temp_ny == 1) {
							if (pipe->CanEnter() && (GetAsyncKeyState(VK_DOWN) & 0x8000)) {
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
						if (temp_ny == -1) { // Tông từ dưới lên
							lucky->Hit(); // Bất kể to hay nhỏ, cứ đụng là chuyển trạng thái!
						}
					}
				}

				// 2. ĐỤNG NỀN TẢNG 1 CHIỀU (Chỉ cản khi rơi từ trên xuống)
				else if (dynamic_cast<Platform*>(e)) {
					if (temp_ny == 1) {
						if (t < min_ty) {
							min_ty = t;
							ny_col = temp_ny;
						}
					}
				}
				// 3. ĐỤNG QUÁI VẬT TRỤC DỌC
				else if (Enemy* enemy = dynamic_cast<Enemy*>(e)) {
					if (!enemy->IsDied()) {
						if (temp_ny == 1) {
							// Rơi xuống đụng enemy
							vy = MARIO_JUMP_SPEED_Y * 0.5f; // Nảy nhẹ lên
							OutputDebugStringA("Enemy stomped!\n");
							enemy->SetDied(true);
						}
						else if (temp_ny == -1) {
							OutputDebugStringA("Mario damaged by enemy\n");
							// Nhảy lên đụng enemy
							TakeDamage();
						}
					}
				}
				// 4. ĐỤNG BUFF
				else if (Buff* buff = dynamic_cast<Buff*>(e)) {
					if (lives < 2)
					{
						lives = 2;
						SetBig(true);

						OutputDebugStringA("Mario became BIG\n");
					}

					buff->Delete();
				}
				// 5. CHẠM CỜ
				else if (Flag* flag = dynamic_cast<Flag*>(e)) {
					GameManager::GetInstance()->SetGameWin(true);
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

	// XỬ LÝ GAME OVER
	if (IsDied())
	{
		GameManager::GetInstance()->SetGameOver(true);
		std::printf("game over\n");
	}
}

void Mario::Render()
{
	Animation* ani = NULL;
	bool isSkidding =
		(vx > 0 && nx < 0) ||
		(vx < 0 && nx > 0);

	//mario chết
	if (isDead)
	{
		ani = Animations::GetInstance()->Get(108);

		if (ani != NULL)
			ani->Render(x, y);

		return;
	}

	//mario lớn
	if (isBig)
	{
		// Jump
		if (!isOnGround)
		{
			if (nx > 0)
				ani = Animations::GetInstance()->Get(204);
			else
				ani = Animations::GetInstance()->Get(205);
		}
		else
		{
			// Skid
			if (isSkidding)
			{
				if (nx > 0)
					ani = Animations::GetInstance()->Get(207);
				else
					ani = Animations::GetInstance()->Get(206);
			}

			// Idle
			else if (vx == 0.0f)
			{
				if (nx > 0)
					ani = Animations::GetInstance()->Get(200);
				else
					ani = Animations::GetInstance()->Get(201);
			}

			// Run
			else
			{
				if (nx > 0)
					ani = Animations::GetInstance()->Get(202);
				else
					ani = Animations::GetInstance()->Get(203);
			}
		}
	}

	//mario nhỏ
	else
	{
		// Jump
		if (!isOnGround)
		{
			if (nx > 0)
				ani = Animations::GetInstance()->Get(104);
			else
				ani = Animations::GetInstance()->Get(105);
		}
		else
		{
			// Skid
			if (isSkidding)
			{
				if (nx > 0)
					ani = Animations::GetInstance()->Get(107);
				else
					ani = Animations::GetInstance()->Get(106);
			}

			// Idle
			else if (vx == 0.0f)
			{
				if (nx > 0)
					ani = Animations::GetInstance()->Get(100);
				else
					ani = Animations::GetInstance()->Get(101);
			}

			// Run
			else
			{
				if (nx > 0)
					ani = Animations::GetInstance()->Get(102);
				else
					ani = Animations::GetInstance()->Get(103);
			}
		}
	}
	//mario ô chớp chớp khi bất tử
	if (untouchable)
	{
		if ((GetTickCount64() / 100) % 2 == 0)
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
	if (isDead)
		return;

	isDead = true;

	vx = 0;
	vy = 0.2f;

	deathStart = GetTickCount64();

	OutputDebugStringA("Mario died\n");
}

void Mario::TakeDamage()
{
	// Đang bất tử
	if (untouchable || isDead)
		return;

	// Mario to -> nhỏ lại
	if (isBig)
	{
		lives = 1;
		SetBig(false);
		untouchable = true;
		untouchableStart = GetTickCount64();

		OutputDebugStringA("Mario shrinked + invincible\n");
	}

	// Mario nhỏ -> chết
	else
	{
		lives = 0;
		Die();
	}
}