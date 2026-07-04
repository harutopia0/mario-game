#pragma once
#include "engine/core/GameObject.h"
#define BUFF_WIDTH 16.0f
#define BUFF_HEIGHT 16.0f
class Buff : public GameObject
{
  private:
    float width, height;
    bool isSprouting;
    float sproutStartY;
    float sproutTargetY;

  public:
    Buff(float x, float y, int animationId);
    void StartSprouting(float startY);
    void GetBoundingBox(float &left, float &top, float &right, float &bottom);
    void Update(DWORD dt, vector<GameObject *> *coObjects);
    void Render() override;
    int animationId;

  public:
    int GetAnimationId() const
    {
        return animationId;
    }
};
