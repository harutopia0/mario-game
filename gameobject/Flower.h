#pragma once

#include "../core/GameObject.h"
#include "../animation/Animation.h"
#include "../animation/Animations.h"

#define ID_ANI_TULIP 110
#define TULIP_WIDTH 12
#define TULIP_BBOX_WIDTH  12
#define TULIP_BBOX_HEIGHT 20

class Tulip : public GameObject {
public:
	Tulip(float x, float y) : GameObject(x, y) {}
	void Render();
	void Update(DWORD dt, vector<GameObject*>* coObjects) {}
	void GetBoundingBox(float& l, float& t, float& r, float& b);
};