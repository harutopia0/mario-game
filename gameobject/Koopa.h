#pragma once
#include "Enemy.h"

#define KOOPA_WALKING_SPEED -0.03f
#define KOOPA_SPINNING_SPEED 0.25f
#define KOOPA_JUMP_SPEED 0.31f

// Koopa types
#define KOOPA_TYPE_GREEN        1
#define KOOPA_TYPE_GREEN_FLYING 2
#define KOOPA_TYPE_RED          3

// Koopa states
#define KOOPA_STATE_WALKING        100
#define KOOPA_STATE_SHELL          200
#define KOOPA_STATE_SHELL_SPINNING 300
#define KOOPA_STATE_SHELL_SHAKING  400
#define KOOPA_STATE_JUMPING        500
#define KOOPA_STATE_DIE            600

// Timeouts
#define KOOPA_WAKE_UP_TIME         5000
#define KOOPA_SHAKE_TIME           3000
#define KOOPA_JUST_SHELLED_TIME    200  // ms: cooldown sau khi vừa vào SHELL, không bị kick ngay
#define KOOPA_KICK_COOLDOWN        500  // ms: sau khi Mario đá, shell không damage Mario

// Cliff detection threshold for Red Koopa
#define KOOPA_CLIFF_CHECK_AHEAD    18.0f

class Koopa : public Enemy
{
private:
	int state;
	int type;
	ULONGLONG shellTimeStart;
	ULONGLONG kickCooldownStart; // thời điểm Mario vừa kick shell

public:
	Koopa(float x, float y, int type = KOOPA_TYPE_GREEN);
	virtual void Update(DWORD dt, vector<GameObject*>* coObjects) override;
	virtual void Render() override;
	virtual void OnStomped(Mario* mario) override;
	virtual void OnCollision(GameObject* obj) override;
	
	void Kick(int direction);
	int GetState() const { return state; }
	int GetType() const { return type; }

	// Trả về true nếu shell vừa được tạo (trong KOOPA_JUST_SHELLED_TIME ms)
	// Dùng để tránh AABB check kick shell ngay sau khi dẫm Koopa
	bool IsJustShelled() const {
		return (state == KOOPA_STATE_SHELL || state == KOOPA_STATE_SHELL_SHAKING)
			&& (GetTickCount64() - shellTimeStart < KOOPA_JUST_SHELLED_TIME);
	}

	// Trả về true nếu shell đang trong cooldown sau khi Mario kick (không thể gây dame cho Mario)
	bool IsKickedCooldown() const {
		return (GetTickCount64() - kickCooldownStart < KOOPA_KICK_COOLDOWN);
	}

	void SetKickedCooldown() { kickCooldownStart = GetTickCount64(); }

private:
	bool CheckCliffAhead(vector<GameObject*>* coObjects);
};
