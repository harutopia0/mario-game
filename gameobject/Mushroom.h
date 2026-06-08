#pragma once

#include "../core/GameObject.h"

#define ID_ANI_MUSHROOM 120
#define MUSHROOM_WIDTH 16
#define MUSHROOM_BBOX_WIDTH  25
#define MUSHROOM_BBOX_HEIGHT 16

class Mushroom : public GameObject {
public:
	Mushroom(float x, float y) : GameObject(x, y) {}
	void Render();
	void Update(DWORD dt, vector<GameObject*>* coObjects) {}
	void GetBoundingBox(float& l, float& t, float& r, float& b);
};