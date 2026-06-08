#pragma once
#include "../core/GameObject.h"

#define COIN_BBOX_WIDTH 10
#define COIN_BBOX_HEIGHT 16

class CCoin : public GameObject
{
public:
	CCoin(float x, float y);
	virtual void Render();
	virtual void Update(DWORD dt, vector<GameObject*>* coObjects = NULL);
	virtual void GetBoundingBox(float& l, float& t, float& r, float& b);
};
