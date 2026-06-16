#pragma once
#include "Enemy.h"

#define GOOMBA_WALKING_SPEED -0.03f
#define GOOMBA_DIE_REVERSE_FACTOR_X 1.7f
#define GOOMBA_DIE_REVERSE_FACTOR_Y 0.4f

// Goomba types
#define GOOMBA_TYPE_NORMAL 1

// States
#define GOOMBA_STATE_WALKING       100
#define GOOMBA_STATE_FLAT          200
#define GOOMBA_STATE_DIE_REVERSE   201

// Timeouts
#define GOOMBA_FLAT_TIMEOUT  500

class Goomba : public Enemy
{
private:
	int state;
	int type;
	ULONGLONG flatTimeStart;

public:
	Goomba(float x, float y, int type = GOOMBA_TYPE_NORMAL);
	virtual void Update(DWORD dt, vector<GameObject*>* coObjects) override;
	virtual void Render() override;
	virtual void OnStomped(Mario* mario) override;
	int GetType() const { return type; }
};
