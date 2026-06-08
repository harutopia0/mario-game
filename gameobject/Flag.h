#pragma once
#include "../core/GameObject.h"
#define BUFF_WIDTH 16.0f
#define BUFF_HEIGHT 16.0f

class Flag : public GameObject
{
private:
	bool isVisited;
	float width, height;
public:
	Flag(float x, float y);
	void GetBoundingBox(float& left, float& top, float& right, float& bottom);
	void Render() override;
	void SetVisited() { isVisited = true; }
	bool GetVisited() const { return isVisited; }
	void Update(DWORD dt, vector<GameObject*>* coObjects) override;
};