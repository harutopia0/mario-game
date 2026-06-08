#include "Goomba.h"
#include "../animation/Animations.h"

CGoomba::CGoomba(float x, float y) : Enemy(x, y, GOOMBA_BBOX_WIDTH, GOOMBA_BBOX_HEIGHT)
{
	this->state = GOOMBA_STATE_WALKING;
	this->vx = -GOOMBA_WALKING_SPEED;
}

void CGoomba::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	left = x;
	top = y;
	right = x + GOOMBA_BBOX_WIDTH;

	if (state == GOOMBA_STATE_DIE)
		bottom = y + GOOMBA_BBOX_HEIGHT_DIE;
	else
		bottom = y + GOOMBA_BBOX_HEIGHT;
}

void CGoomba::Update(DWORD dt, vector<GameObject*>* coObjects)
{
	Enemy::Update(dt, coObjects);
}

void CGoomba::Render()
{
	int aniId = 401; // Default walking
	if (state == GOOMBA_STATE_DIE) 
		aniId = 402;

	Animations::GetInstance()->Get(aniId)->Render(x, y);
}

void CGoomba::SetState(int state)
{
	Enemy::SetState(state);
	switch (state)
	{
		case GOOMBA_STATE_DIE:
			y += (GOOMBA_BBOX_HEIGHT - GOOMBA_BBOX_HEIGHT_DIE) / 2;
			vx = 0;
			vy = 0;
			break;
		case GOOMBA_STATE_WALKING: 
			vx = -GOOMBA_WALKING_SPEED;
			break;
	}
}
