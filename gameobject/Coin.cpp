#include "Coin.h"
#include "../animation/Animations.h"

CCoin::CCoin(float x, float y) : GameObject(x, y)
{
}

void CCoin::Render()
{
	Animations::GetInstance()->Get(601)->Render(x, y);
}

void CCoin::Update(DWORD dt, vector<GameObject*>* coObjects)
{
}

void CCoin::GetBoundingBox(float& l, float& t, float& r, float& b)
{
	l = x;
	t = y;
	r = x + COIN_BBOX_WIDTH;
	b = y + COIN_BBOX_HEIGHT;
}
