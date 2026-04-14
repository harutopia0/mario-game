#pragma once
#include <Windows.h>

class GameObject
{
protected:
    float x, y;
    float vx, vy;
    int nx;

public:
    GameObject(float x = 0.0f, float y = 0.0f);
    virtual void Update(DWORD dt);
    virtual void Render() = 0;
    virtual ~GameObject() {}
};