#include "Koopa.h"
#include "../audio/AudioManager.h"
#include "../gameobject/Mario.h"
#include "../gameobject/Block.h"
#include "../gameobject/Platform.h"
#include "../physics/Collision.h"
#include "../animation/Animations.h"
#include <cstdlib>

Koopa::Koopa(float x, float y, int type) : Enemy(x, y, 312)
{
	this->state = KOOPA_STATE_WALKING;
	this->type = type;
	this->vx = KOOPA_WALKING_SPEED;
	this->nx = -1;
	this->width = 16.0f;
	this->height = 27.0f;
	this->shellTimeStart = 0;
	this->kickCooldownStart = 0;

	if (type == KOOPA_TYPE_GREEN_FLYING)
	{
		// Flying Koopa bắt đầu ở trạng thái nhảy
		this->state = KOOPA_STATE_JUMPING;
		this->vy = KOOPA_JUMP_SPEED;
		this->height = 29.0f;
	}
}

bool Koopa::CheckCliffAhead(vector<GameObject*>* coObjects)
{
	// Kiểm tra xem phía trước có mặt đất không
	// Nếu không có đất => là vực => cần quay đầu
	float checkX = (vx < 0) ? x - KOOPA_CLIFF_CHECK_AHEAD : x + width + KOOPA_CLIFF_CHECK_AHEAD;
	float checkY = y - 2.0f; // Kiểm tra ngay dưới chân Koopa

	for (GameObject* obj : *coObjects)
	{
		if (obj == this || obj->IsDeleted()) continue;
		Block* block = dynamic_cast<Block*>(obj);
		if (block && !dynamic_cast<Platform*>(block))
		{
			float sl, st, sr, sb;
			block->GetBoundingBox(sl, st, sr, sb);
			// Kiểm tra xem block có nằm ở vị trí phía trước và bên dưới không
			if (checkX >= sl && checkX <= sr && checkY >= st && checkY <= sb)
			{
				return false; // Có mặt đất, không phải vực
			}
		}
	}
	return true; // Không có đất phía trước => vực
}

void Koopa::Update(DWORD dt, vector<GameObject*>* coObjects)
{
	if (isDeleted) return;

	// Trạng thái chết (lật ngược rơi khỏi màn hình)
	if (state == KOOPA_STATE_DIE)
	{
		vy += ENEMY_GRAVITY * dt;
		x += vx * dt;
		y += vy * dt;
		if (y < -100.0f) // Rơi ra ngoài màn hình
		{
			Delete();
		}
		return;
	}

	// Xử lý trạng thái mai rùa nằm im
	if (state == KOOPA_STATE_SHELL)
	{
		vx = 0.0f;
		
		ULONGLONG elapsed = GetTickCount64() - shellTimeStart;
		
		// Chuyển sang rung lắc trước khi tỉnh dậy
		if (elapsed >= KOOPA_SHAKE_TIME && state == KOOPA_STATE_SHELL)
		{
			state = KOOPA_STATE_SHELL_SHAKING;
		}
	}

	// Xử lý trạng thái rung lắc (sắp tỉnh dậy)
	if (state == KOOPA_STATE_SHELL_SHAKING)
	{
		vx = 0.0f;
		ULONGLONG elapsed = GetTickCount64() - shellTimeStart;
		if (elapsed >= KOOPA_WAKE_UP_TIME)
		{
			// Tỉnh dậy quay lại đi bộ
			if (type == KOOPA_TYPE_GREEN_FLYING)
			{
				// Flying Koopa tỉnh dậy thành Green Koopa đi bộ thường
				state = KOOPA_STATE_WALKING;
				type = KOOPA_TYPE_GREEN; // Mất cánh vĩnh viễn
			}
			else
			{
				state = KOOPA_STATE_WALKING;
			}
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
					// Đâm quái khác - chết lật ngược
					enemy->OnStomped(NULL);
				}
			}
		}
	}

	// Red Koopa: Kiểm tra vực trước mặt khi đi bộ
	if (type == KOOPA_TYPE_RED && state == KOOPA_STATE_WALKING)
	{
		if (vy == 0 && CheckCliffAhead(coObjects))
		{
			// Phát hiện vực, quay đầu
			vx = -vx;
			nx = -nx;
		}
	}

	// Flying Koopa: Nhảy lại khi chạm đất
	if (type == KOOPA_TYPE_GREEN_FLYING && state == KOOPA_STATE_JUMPING)
	{
		// Đặc biệt: để Enemy::Update xử lý trọng lực và va chạm
		// Sau khi chạm đất (vy == 0), nhảy lại
	}

	Enemy::Update(dt, coObjects);

	// Flying Koopa: Nhảy lại khi chạm đất
	if (type == KOOPA_TYPE_GREEN_FLYING && state == KOOPA_STATE_JUMPING)
	{
		if (vy == 0)
		{
			vy = KOOPA_JUMP_SPEED;
		}
	}
}

void Koopa::Render()
{
	if (isDeleted) return;

	int aniId;
	float drawX = x;

	if (type == KOOPA_TYPE_GREEN || type == KOOPA_TYPE_GREEN_FLYING)
	{
		if (state == KOOPA_STATE_WALKING)
		{
			aniId = (nx < 0) ? 312 : 330; // Walk left / right
		}
		else if (state == KOOPA_STATE_JUMPING)
		{
			aniId = (nx < 0) ? 350 : 351; // Jump left / right
		}
		else if (state == KOOPA_STATE_SHELL)
		{
			aniId = 313; // Shell static
		}
		else if (state == KOOPA_STATE_SHELL_SHAKING)
		{
			// Rung lắc trước khi tỉnh dậy
			drawX += (rand() % 3) - 1;
			aniId = 315; // Shell shaking
		}
		else if (state == KOOPA_STATE_SHELL_SPINNING)
		{
			aniId = 314; // Shell spinning
		}
		else if (state == KOOPA_STATE_DIE)
		{
			aniId = 331; // Die (flipped)
		}
		else
		{
			aniId = 312; // Default walk left
		}
	}
	else // RED KOOPA
	{
		if (state == KOOPA_STATE_WALKING)
		{
			aniId = (nx < 0) ? 340 : 341; // Red walk left / right
		}
		else if (state == KOOPA_STATE_SHELL)
		{
			aniId = 342; // Red shell static
		}
		else if (state == KOOPA_STATE_SHELL_SHAKING)
		{
			drawX += (rand() % 3) - 1;
			aniId = 344; // Red shell shaking
		}
		else if (state == KOOPA_STATE_SHELL_SPINNING)
		{
			aniId = 343; // Red shell spinning
		}
		else if (state == KOOPA_STATE_DIE)
		{
			aniId = 331; // Die (reuse green die sprite, looks similar)
		}
		else
		{
			aniId = 340; // Default red walk left
		}
	}

	Animation* anim = Animations::GetInstance()->Get(aniId);
	if (anim == nullptr) return;

	// Offset cố định từ walking sprite (28px) - hitbox (27px) = 1px
	// Dùng hằng số để shell/walking/jumping đều căn chỉnh nhất quán
	anim->Render(drawX, y - 1.0f);
}

void Koopa::OnStomped(Mario* mario)
{
	if (state == KOOPA_STATE_WALKING || state == KOOPA_STATE_JUMPING)
	{
		if (type == KOOPA_TYPE_GREEN_FLYING)
		{
			// Rùa bay bị dẫm: mất cánh, tiếp tục đi bộ bình thường (không vào shell)
			type = KOOPA_TYPE_GREEN;
			state = KOOPA_STATE_WALKING;
			height = 27.0f;
			vx = KOOPA_WALKING_SPEED; // Đi theo hướng mặc định
			vy = 0.0f;
			nx = -1;
		}
		else
		{
			// Koopa thường bị dẫm → vào mai
			state = KOOPA_STATE_SHELL;
			shellTimeStart = GetTickCount64();
			height = 15.0f;
			vx = 0.0f;
			vy = 0.0f;
		}

		AudioManager::GetInstance()->PlaySFX("stomp");
	}
	else if (state == KOOPA_STATE_SHELL || state == KOOPA_STATE_SHELL_SHAKING)
	{
		// Dẫm lên mai rùa đang nằm im => đá nó đi
		if (mario != NULL)
		{
			int dir = (mario->GetX() < x) ? 1 : -1;
			Kick(dir);
		}
		else
		{
			// Bị đâm bởi mai rùa xoay khác => chết
			state = KOOPA_STATE_DIE;
			vy = KOOPA_JUMP_SPEED * 0.5f;
			vx = (nx > 0) ? -0.05f : 0.05f;
			layer = LAYER_BACKGROUND;
		}
	}
	else if (state == KOOPA_STATE_SHELL_SPINNING)
	{
		// Dẫm lên mai rùa đang quay => dừng nó lại
		state = KOOPA_STATE_SHELL;
		shellTimeStart = GetTickCount64();
		vx = 0.0f;

		AudioManager::GetInstance()->PlaySFX("stomp");
	}
}

void Koopa::Kick(int direction)
{
	SetKickedCooldown(); // Bật cooldown 500ms, shell không thể damage Mario trong thời gian này
	state = KOOPA_STATE_SHELL_SPINNING;
	vx = direction * KOOPA_SPINNING_SPEED;
	nx = direction;

	AudioManager::GetInstance()->PlaySFX("kick");
}

void Koopa::OnCollision(GameObject* obj)
{
}
