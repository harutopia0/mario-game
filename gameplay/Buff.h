#pragma once
#include "GameObject.h"
class Buff : public GameObject
{
private:
		float width, height;
public:
	Buff(float x, float y, float width, float height, int type);
		void GetBoundingBox(float& left, float& top, float& right, float& bottom);
		void Update(DWORD dt, vector<GameObject*>* coObjects);
		void Render() override;
};

