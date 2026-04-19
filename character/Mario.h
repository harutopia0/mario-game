#pragma once
#include "../gameplay/GameObject.h"

#define MARIO_WIDTH 32.0f  
#define MARIO_HEIGHT 32.0f
#define MARIO_JUMP_SPEED_Y 1.0f
#define MARIO_GRAVITY -0.002f 

class Mario : public GameObject
{
public:
    Mario(float x, float y);
    void GetBoundingBox(float& left, float& top, float& right, float& bottom);
    void Update(DWORD dt, vector<GameObject*>* coObjects);
    void Render() override;
};