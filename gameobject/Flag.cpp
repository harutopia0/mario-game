#include "Flag.h"
#include "../animation/Animations.h"
#include "../gameplay/GameManager.h"

Flag::Flag(float x, float y)
	: GameObject(x, y){
	width = 30.0f;
	height = 151.0f;

	vx = 0.0f;
	vy = 0.0f;

	isStatic = true;
	isDeleted = false;
	isVisited = false;
}

void Flag::Render() {
	Animation* ani = NULL;

	if (isVisited)
	{
		ani = Animations::GetInstance()->Get(900); // flag đã chạm (tạm thời dùng chung)
	}
	else
	{
		ani = Animations::GetInstance()->Get(900); // flag bình thường
	}

	if (ani) ani->Render(x, y);
}

void Flag::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	left = x;
	top = y;
	right = x + width;
	bottom = y + height;
}

void Flag::Update(DWORD dt, vector<GameObject*>* coObjects)
{
	if (isVisited)
	{
		GameManager::GetInstance()->SetGameWin(true);
		std::printf("Game Win!\n");
	}
}


