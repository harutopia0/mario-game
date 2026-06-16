#include "Goomba.h"
#include "../audio/AudioManager.h"
#include "../animation/Animations.h"

Goomba::Goomba(float x, float y) : Enemy(x, y, 310)
{
	this->state = GOOMBA_STATE_WALKING;
	this->vx = GOOMBA_WALKING_SPEED;
	this->nx = -1;
	this->flatTimeStart = 0;
	this->width = 16.0f;
	this->height = 16.0f;
}

void Goomba::Update(DWORD dt, vector<GameObject*>* coObjects)
{
	if (isDeleted) return;

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

	Enemy::Update(dt, coObjects);
}

void Goomba::Render()
{
	if (isDeleted) return;

	int aniId = 310; // WALKING
	if (state == GOOMBA_STATE_FLAT)
	{
		aniId = 311; // FLAT
	}

	Animations::GetInstance()->Get(aniId)->Render(x, y);
}

void Goomba::OnStomped(Mario* mario)
{
	if (state == GOOMBA_STATE_FLAT) return;

	state = GOOMBA_STATE_FLAT;
	flatTimeStart = GetTickCount64();
	vx = 0.0f;
	vy = 0.0f;
	
	// Không va chạm nữa
	layer = LAYER_BACKGROUND;

	// Phát âm thanh dẫm
	AudioManager::GetInstance()->PlaySFX("stomp");
}
