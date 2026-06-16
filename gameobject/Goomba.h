#pragma once
#include "Enemy.h"

#define GOOMBA_WALKING_SPEED -0.03f

#define GOOMBA_STATE_WALKING 100
#define GOOMBA_STATE_FLAT    200

#define GOOMBA_FLAT_TIMEOUT  500

class Goomba : public Enemy
{
private:
	int state;
	ULONGLONG flatTimeStart;

public:
	Goomba(float x, float y);
	virtual void Update(DWORD dt, vector<GameObject*>* coObjects) override;
	virtual void Render() override;
	virtual void OnStomped(Mario* mario) override;
};
