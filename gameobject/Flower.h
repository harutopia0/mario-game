#pragma once

#include "../core/GameObject.h"
#include "../animation/Animation.h"
#include "../animation/Animations.h"

#define ID_ANI_FLOWER 110
#define FLOWER_WIDTH 12
#define FLOWER_BBOX_WIDTH  12
#define FLOWER_BBOX_HEIGHT 20

class CFlower : public CGameObject {
public:
	Tulip(float x, float y) : GameObject(x, y) {}
	void Render();
	void Update(DWORD dt, vector<GameObject*>* coObjects) {}
	void GetBoundingBox(float& l, float& t, float& r, float& b);
};