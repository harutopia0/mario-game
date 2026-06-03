#pragma once
#include "../core/GameObject.h"
#include "../gameobject/Platform.h"

#define MARIO_SMALL_WIDTH 13.0f
#define MARIO_SMALL_HEIGHT 16.0f
#define MARIO_BIG_WIDTH 15.0f
#define MARIO_BIG_HEIGHT 27.0f
#define MARIO_JUMP_SPEED_Y 1.0f
#define MARIO_GRAVITY -0.002f 

class Mario : public GameObject
{
private:
	float width, height;
	int lives;
	bool isBig;
	bool isDead;
	float pMeterValue;
	int pMeterLevel;
	DWORD pMeterTimer;
public:
	DWORD deathStart;
	bool isOnGround;
	float ax;
	Mario(float x, float y);
	void GetBoundingBox(float& left, float& top, float& right, float& bottom);
	void Update(DWORD dt, vector<GameObject*>* coObjects);
	void Render() override;
	bool IsDied() const
	{
		return isDead;
	}
	void SetDied(bool died)
	{
		isDead = died;
	}
	void SetBig(bool big);
	bool IsBig() const { return isBig; }
	void Die();

	DWORD untouchableStart;
	bool untouchable;
	void TakeDamage();
};