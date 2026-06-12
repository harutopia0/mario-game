#pragma once
#include "../core/GameObject.h"
#define BUFF_WIDTH 16.0f
#define BUFF_HEIGHT 16.0f
class Buff : public GameObject
{
private:
		float width, height;
public:
	Buff(float x, float y, int animationId);
		void GetBoundingBox(float& left, float& top, float& right, float& bottom);
		void Update(DWORD dt, vector<GameObject*>* coObjects);
		void Render() override;
    int animationId;
public:
	int GetAnimationId() const { return animationId; }
};

