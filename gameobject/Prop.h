#pragma once
#include "../core/GameObject.h"

class Prop : public GameObject
{
protected:
    int spriteId;
    float width;
    float height;
public:
    Prop(float x, float y, int spriteId, float width, float height);
    virtual void Render();
    virtual void Update(DWORD dt, vector<GameObject*>* coObjects) {}
    virtual void GetBoundingBox(float& l, float& t, float& r, float& b);
    virtual int IsBlocking() { return 0; }
    virtual int IsCollidable() { return 0; }
};
