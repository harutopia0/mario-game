#pragma once
#include "../gameplay/GameObject.h"

#define MARIO_WIDTH 32.0f  
#define MARIO_HEIGHT 32.0f
#define MARIO_JUMP_SPEED_Y 1.0f
#define MARIO_GRAVITY -0.002f 

class Mario : public GameObject
{
private:
	float width, height;
    int lives;
public:
    bool isOnGround;
    float ax;
    Mario(float x, float y, float width, float height);
    void GetBoundingBox(float& left, float& top, float& right, float& bottom);
    void Update(DWORD dt, vector<GameObject*>* coObjects);
    void Render() override;
	void OnCollision(GameObject* obj);
	bool IsDied() const { return lives <= 0; }
};