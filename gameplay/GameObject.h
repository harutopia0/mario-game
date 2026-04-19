#pragma once
#include <Windows.h>
#include <algorithm>
#include <vector>
using namespace std;

class GameObject
{
protected:
    float x, y;
    float vx, vy;
    int nx;

public:
    GameObject(float x = 0.0f, float y = 0.0f);
    virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom) = 0;
    void SweptAABB(
        float ml, float mt, float mr, float mb,
        float dx, float dy,
        float sl, float st, float sr, float sb,
        float& t, float& nx, float& ny);
    virtual void Update(DWORD dt, vector<GameObject*>* coObjects);
    virtual void Render() = 0;
    virtual void RenderBoundingBox();
    virtual ~GameObject() {}
};