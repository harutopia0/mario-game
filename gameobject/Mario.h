#pragma once
#include "../core/GameObject.h"
#include "../gameobject/Platform.h"

#define MARIO_SMALL_WIDTH 13.0f
#define MARIO_SMALL_HEIGHT 16.0f
#define MARIO_BIG_WIDTH 15.0f
#define MARIO_BIG_HEIGHT 27.0f

#define MARIO_JUMP_SPEED_Y      0.27f 
#define MARIO_JUMP_DEFLECT_SPEED 0.1f
#define MARIO_GRAVITY           -0.00067f
#define MARIO_WALKING_SPEED		0.15f
#define MARIO_ACCEL_WALK_X		0.0005f 
#define MARIO_FRICTION			0.0004f

// Thời gian bất tử
#define MARIO_UNTOUCHABLE_TIME 5000

// ĐỊNH NGHĨA STEP THỜI GIAN GIỮA CÁC MỨC PMETER (mili-giây)
#define PMETER_STEP_UP_TIME     150
#define PMETER_STEP_DOWN_TIME    80

class MarioInputHandler;

class Mario : public GameObject
{
private:
	float width, height;
	bool isBig;
	bool isFire;
	bool isDead;
	float pMeterValue;
	int pMeterLevel;
	DWORD pMeterTimer;
public:
	DWORD deathStart;
	bool isOnGround;

	bool isPipeAnimating;
	float pipeDestX;
	float pipeDestY;
	float pipeEnterStartY;

	bool isPressingDown;

	float ax;
	MarioInputHandler* inputHandler;

	Mario(float x, float y, bool isBig = false, bool isFire = false);
	~Mario();
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
	
	void SetFire(bool fire);
	bool IsFire() const { return isFire; }
	void ShootFireball();
	void ShootFireBlast();
	DWORD lastShootTime;

	void Die();

	void SetAccelX(float ax);
	void SetDirection(int nx);
	void Jump();
	void SetHoldingJump(bool holding);
	void SetPressingDown(bool pressing);

	DWORD untouchableStart;
	DWORD untouchableDuration;
	bool untouchable;
	bool isStarInvincible;
	void TakeDamage();
};