#pragma once
#include "../core/GameObject.h"

#define ENEMY_GRAVITY -0.002f
class Enemy : public GameObject
{
private:
	float width, height;
	int animationId;
	bool died;
	bool isFreezed;
public:
	Enemy(float x, float y, int animationId);
	void GetBoundingBox(float& left, float& top, float& right, float& bottom);
	void Update(DWORD dt, vector<GameObject*>* coObjects);
	void Render() override;
	void OnCollision(GameObject* obj);
	bool IsDied() const { return died; }
	bool SetDied(bool died) {
		this->died = died;
		return this->died;
	}
	bool IsFreezed() const { return isFreezed; }
	void SetFreezed(bool frozen) { this->isFreezed = frozen; }
};

