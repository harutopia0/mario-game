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
    bool isDeleted;

public:
    //địa chỉ trong grid của object
    int gridRow;
    int gridCol;
    float GetX() { return x; }
    float GetY() { return y; }
    bool isStatic = false;
    GameObject(float x = 0.0f, float y = 0.0f);
    virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom) = 0;
    virtual void Update(DWORD dt, vector<GameObject*>* coObjects);
    virtual void Render() = 0;
    virtual void RenderBoundingBox();
    virtual void Delete();
    bool IsDeleted() const;
    virtual ~GameObject() {}
};