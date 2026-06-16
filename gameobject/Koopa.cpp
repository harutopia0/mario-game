#include "Koopa.h"
#include "../audio/AudioManager.h"
#include "../gameobject/Mario.h"
#include "../physics/Collision.h"
#include "../animation/Animations.h"
#include <cstdlib>

Koopa::Koopa(float x, float y) : Enemy(x, y, 312)
{
	this->state = KOOPA_STATE_WALKING;
	this->vx = KOOPA_WALKING_SPEED;
	this->nx = -1;
	this->width = 16.0f;
	this->height = 27.0f;
	this->shellTimeStart = 0;
}

void Koopa::Update(DWORD dt, vector<GameObject*>* coObjects)
{
	if (isDeleted) return;

	if (state == KOOPA_STATE_SHELL)
	{
		vx = 0.0f;
		
		// Tỉnh dậy sau 5 giây
		ULONGLONG elapsed = GetTickCount64() - shellTimeStart;
		if (elapsed >= KOOPA_WAKE_UP_TIME)
		{
			// Tỉnh dậy quay lại đi bộ
			state = KOOPA_STATE_WALKING;
			height = 27.0f;
			vx = KOOPA_WALKING_SPEED;
			nx = -1;
		}
	}

	// Va chạm đâm chết quái khác khi mai rùa xoay
	if (state == KOOPA_STATE_SHELL_SPINNING)
	{
		float l1, t1, r1, b1;
		GetBoundingBox(l1, t1, r1, b1);

		for (GameObject* obj : *coObjects)
		{
			if (obj == this || obj->IsDeleted()) continue;
			Enemy* enemy = dynamic_cast<Enemy*>(obj);
			if (enemy && !enemy->IsDied())
			{
				float l2, t2, r2, b2;
				enemy->GetBoundingBox(l2, t2, r2, b2);
				if (r1 > l2 && l1 < r2 && b1 > t2 && t1 < b2)
				{
					// Đâm quái khác bẹp luôn
					enemy->OnStomped(NULL);
				}
			}
		}
	}

	Enemy::Update(dt, coObjects);
}

void Koopa::Render()
{
	if (isDeleted) return;

	int aniId = 312; // KOOPA WALK
	float drawX = x;

	if (state == KOOPA_STATE_SHELL)
	{
		ULONGLONG elapsed = GetTickCount64() - shellTimeStart;
		if (elapsed >= KOOPA_SHAKE_TIME)
		{
			// Rung lắc trước khi tỉnh dậy
			drawX += (rand() % 3) - 1;
			aniId = 315; // Shaking shell (feet flashing)
		}
		else
		{
			aniId = 313; // Static shell
		}
	}
	else if (state == KOOPA_STATE_SHELL_SPINNING)
	{
		aniId = 314; // Spinning shell
	}

	// Vẽ lật sprite theo hướng nx
	Animations::GetInstance()->Get(aniId)->Render(drawX, y, nx);
}

void Koopa::OnStomped(Mario* mario)
{
	if (state == KOOPA_STATE_WALKING)
	{
		state = KOOPA_STATE_SHELL;
		shellTimeStart = GetTickCount64();
		height = 15.0f;
		vx = 0.0f;

		// Phát âm thanh stomp
		AudioManager::GetInstance()->PlaySFX("stomp");
	}
	else if (state == KOOPA_STATE_SHELL)
	{
		// Nếu đang đứng im ở dạng mai rùa mà dẫm lên, ta đá nó đi
		if (mario != NULL)
		{
			int dir = (mario->GetX() < x) ? 1 : -1;
			Kick(dir);
		}
	}
	else if (state == KOOPA_STATE_SHELL_SPINNING)
	{
		// Nếu đang quay mà bị dẫm tiếp, ta dừng nó lại
		state = KOOPA_STATE_SHELL;
		shellTimeStart = GetTickCount64();
		vx = 0.0f;

		AudioManager::GetInstance()->PlaySFX("stomp");
	}
}

void Koopa::Kick(int direction)
{
	state = KOOPA_STATE_SHELL_SPINNING;
	vx = direction * KOOPA_SPINNING_SPEED;
	nx = direction;

	// Phát âm thanh đá
	AudioManager::GetInstance()->PlaySFX("kick");
}

void Koopa::OnCollision(GameObject* obj)
{
}
