#include "Koopas.h"
#include "../animation/Animations.h"

CKoopas::CKoopas(float x, float y) : Enemy(x, y, KOOPAS_BBOX_WIDTH, KOOPAS_BBOX_HEIGHT)
{
	this->state = KOOPAS_STATE_WALKING;
	this->vx = -KOOPAS_WALKING_SPEED;
}

void CKoopas::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	left = x;
	top = y;
	right = x + KOOPAS_BBOX_WIDTH;

	if (state == KOOPAS_STATE_WALKING)
		bottom = y + KOOPAS_BBOX_HEIGHT;
	else
		bottom = y + KOOPAS_BBOX_HEIGHT_SHELL;
}

void CKoopas::Update(DWORD dt, vector<GameObject*>* coObjects)
{
	Enemy::Update(dt, coObjects);
}

void CKoopas::Render()
{
	int aniId = 501; // Default walking
	if (state == KOOPAS_STATE_SHELL)
		aniId = 502;
	else if (state == KOOPAS_STATE_SPINNING)
		aniId = 503;

	Animations::GetInstance()->Get(aniId)->Render(x, y);
}

void CKoopas::SetState(int state)
{
	Enemy::SetState(state);
	switch (state)
	{
		case KOOPAS_STATE_SHELL:
			y += (KOOPAS_BBOX_HEIGHT - KOOPAS_BBOX_HEIGHT_SHELL) / 2;
			vx = 0;
			vy = 0;
			break;
		case KOOPAS_STATE_WALKING:
			vx = -KOOPAS_WALKING_SPEED;
			break;
		case KOOPAS_STATE_SPINNING:
			vx = KOOPAS_SHELL_SPEED * nx;
			break;
	}
}
