#pragma once
#include "../core/GameObject.h"

#define WATER_BBOX_WIDTH 29.0f
#define WATER_BBOX_HEIGHT 29.0f

class Water : public GameObject
{
public:
	Water(float x, float y);
	virtual void Render();
	virtual void Update(DWORD dt, vector<GameObject*>* coObjects);
	virtual void GetBoundingBox(float& l, float& t, float& r, float& b);
	virtual int IsCollidable() { return 1; };
	virtual int IsBlocking() { return 0; } // Mario can fall through it
};
