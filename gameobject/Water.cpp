#include "Water.h"
#include "../gameplay/Map.h"
#include "Mario.h"
#include "../animation/Animations.h"
#include "Enemy.h"

Water::Water(float x, float y) : GameObject(x, y)
{
}

void Water::Render()
{
	Animations* animations = Animations::GetInstance();
	if (animations->Get(8001)) {
		animations->Get(8001)->Render(x, y);
	}
}

void Water::Update(DWORD dt, vector<GameObject*>* coObjects)
{
	if (isDeleted) return;
	auto& g_objectList = Map::GetInstance()->GetObjects();

	float wl, wt, wr, wb;
	GetBoundingBox(wl, wt, wr, wb);

	for (UINT i = 0; i < g_objectList.size(); i++)
	{
		GameObject* obj = g_objectList[i];
		if (obj == nullptr || obj->IsDeleted()) continue;

		float ol, ot, o_r, ob;
		obj->GetBoundingBox(ol, ot, o_r, ob);

		if (ol < wr && o_r > wl && ot < wb && ob > wt)
		{
			if (Mario* mario = dynamic_cast<Mario*>(obj))
			{
				if (!mario->IsDied())
				{
					mario->Die();
				}
			}
			else if (Enemy* enemy = dynamic_cast<Enemy*>(obj))
			{
				if (!enemy->IsDied())
				{
					enemy->SetDied(true);
				}
			}
		}
	}
}

void Water::GetBoundingBox(float& l, float& t, float& r, float& b)
{
	Animations* animations = Animations::GetInstance();
	Animation* ani = animations->Get(8001);
	int w = WATER_BBOX_WIDTH;
	int h = WATER_BBOX_HEIGHT;
	if (ani) {
		w = ani->GetWidth();
		h = ani->GetHeight();
	}
	l = x;
	t = y;
	r = l + w;
	b = t + h;
}
