#include "game/entities/Goomba.h"
#include "engine/audio/AudioManager.h"
#include "engine/graphics/Animations.h"
#include "game/entities/Mario.h"
#include "game/scenes/GameManager.h"

Goomba::Goomba(float x, float y, int type) : Enemy(x, y, 310)
{
	this->state = GOOMBA_STATE_WALKING;
	this->type = type;
	this->vx = GOOMBA_WALKING_SPEED;
	this->nx = -1;
	this->flatTimeStart = 0;
	this->width = 16.0f;
	this->height = 16.0f;
}

void Goomba::Update(DWORD dt, vector<GameObject*>* coObjects)
{
	if (isDeleted) return;

	// Xử lý trạng thái chết bẹp
	if (state == GOOMBA_STATE_FLAT)
	{
		vx = 0;
		vy = 0;
		if (GetTickCount64() - flatTimeStart > GOOMBA_FLAT_TIMEOUT)
		{
			Delete();
		}
		return;
	}

	// Xử lý trạng thái chết lật ngược (rơi khỏi màn hình)
	if (state == GOOMBA_STATE_DIE_REVERSE)
	{
		vy += ENEMY_GRAVITY * dt;
		x += vx * dt;
		y += vy * dt;
		if (GetTickCount64() - flatTimeStart > 700)
		{
			Delete();
		}
		return;
	}

	Enemy::Update(dt, coObjects);
}

// Offset cố định được xác định từ walking sprite (18px) - hitbox (16px) = 2px
// Dùng hằng số để mọi state (walking, flat, die) đều căn chỉnh nhất quán
#define GOOMBA_RENDER_OFFSET_Y 2.0f

void Goomba::Render()
{
	if (isDeleted) return;

	int aniId = 310; // Walking
	if (state == GOOMBA_STATE_FLAT)
		aniId = 311; // Flat die
	else if (state == GOOMBA_STATE_DIE_REVERSE)
		aniId = 316; // Die reverse

	// Render với offset cố định: chân sprite luôn khớp bounding box bottom
	Animations::GetInstance()->Get(aniId)->Render(x, y - GOOMBA_RENDER_OFFSET_Y);
}

void Goomba::OnStomped(Mario* mario)
{
	if (state == GOOMBA_STATE_FLAT || state == GOOMBA_STATE_DIE_REVERSE)
		return;

	// Trạng thái bẹp dí (dẫm từ trên xuống)
	if (mario != NULL)
	{
		state = GOOMBA_STATE_FLAT;
		flatTimeStart = GetTickCount64();
		vx = 0.0f;
		vy = 0.0f;
		died = true;

		// Không va chạm nữa
		layer = LAYER_BACKGROUND;

		AudioManager::GetInstance()->PlaySFX("stomp");
		GameManager::GetInstance()->AddScore(100);
		GameManager::GetInstance()->AddKills(1);
	}
	else
	{
		// Bị đâm bởi mai rùa xoay => chết lật ngược
		state = GOOMBA_STATE_DIE_REVERSE;
		flatTimeStart = GetTickCount64();
		vy = GOOMBA_DIE_REVERSE_FACTOR_Y;
		vx = (nx > 0) ? -GOOMBA_WALKING_SPEED * GOOMBA_DIE_REVERSE_FACTOR_X
		              : GOOMBA_WALKING_SPEED * GOOMBA_DIE_REVERSE_FACTOR_X;
		died = true;

		layer = LAYER_BACKGROUND;
		AudioManager::GetInstance()->PlaySFX("stomp");
		GameManager::GetInstance()->AddScore(200);
		GameManager::GetInstance()->AddKills(1);
	}
}
