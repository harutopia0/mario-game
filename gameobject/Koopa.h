#pragma once
#include "Enemy.h"

#define KOOPA_WALKING_SPEED -0.03f
#define KOOPA_SPINNING_SPEED 0.15f

#define KOOPA_STATE_WALKING        100
#define KOOPA_STATE_SHELL          200
#define KOOPA_STATE_SHELL_SPINNING 300

#define KOOPA_WAKE_UP_TIME         5000
#define KOOPA_SHAKE_TIME           3000

class Koopa : public Enemy
{
private:
	int state;
	ULONGLONG shellTimeStart;

public:
	Koopa(float x, float y);
	virtual void Update(DWORD dt, vector<GameObject*>* coObjects) override;
	virtual void Render() override;
	virtual void OnStomped(Mario* mario) override;
	virtual void OnCollision(GameObject* obj) override;
	
	void Kick(int direction);
	int GetState() const { return state; }
};
