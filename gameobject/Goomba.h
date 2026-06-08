#pragma once
#include "Enemy.h"

#define GOOMBA_WALKING_SPEED 0.05f
#define GOOMBA_GRAVITY 0.002f

#define GOOMBA_BBOX_WIDTH 16
#define GOOMBA_BBOX_HEIGHT 14
#define GOOMBA_BBOX_HEIGHT_DIE 7

#define GOOMBA_STATE_WALKING 100
#define GOOMBA_STATE_DIE 200

class CGoomba : public Enemy
{
protected:
	int state;
public:
	CGoomba(float x, float y);
	virtual void SetState(int state);
	virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom);
	virtual void Update(DWORD dt, vector<GameObject*>* coObjects);
	virtual void Render();
};
