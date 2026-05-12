#pragma once

#include "../core/GameObject.h"

#define ID_ANI_PIPE 11
#define PIPE_WIDTH 20
#define GROUND_BBOX_WIDTH 40
#define GROUND_BBOX_HEIGHT 15

class Pipe : public GameObject {
public:
	Pipe(float x, float y) : GameObject(x, y) {}
	void Render();
	void Update(DWORD dt, vector<GameObject*>* coObjects) {}
	void GetBoundingBox(float& l, float& t, float& r, float& b);
};