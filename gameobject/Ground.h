#pragma once

#include "../core/GameObject.h"

#define ID_ANI_GROUND 10
#define GROUND_WIDTH 1
#define GROUND_BBOX_WIDTH 1
#define GROUND_BBOX_HEIGHT 1

class Ground : public GameObject {
public:
	Ground(float x, float y) : GameObject(x, y) {}
	void Render();
	void Update(DWORD dt, vector<GameObject*>* coObjects) {}
	void GetBoundingBox(float& l, float& t, float& r, float& b);
};