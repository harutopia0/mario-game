#pragma once
#include "../gameplay/GameObject.h"

class Flag : public GameObject
{
private:
	bool isVisited;
	float width, height;
public :
	Flag(float x, float y, float width, float height, bool isVisited = false);
	void GetBoundingBox(float& left, float& top, float& right, float& bottom);
	void Render() override;
	void SetVisited() { isVisited = true;}
	void GetVisited() const { return isVisited; }
	void Update(DWORD dt, vector<GameObject*>* coObjects) override {}
};

