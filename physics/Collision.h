#pragma once
#include <vector>
#include <algorithm>
#include "../core/GameObject.h"

using namespace std;

class GameObject;

struct CollisionEvent
{
    GameObject* obj;
    float t, nx, ny;

    CollisionEvent(float t, float nx, float ny, GameObject* obj)
    {
        this->t = t;
        this->nx = nx;
        this->ny = ny;
        this->obj = obj;
    }
};

typedef CollisionEvent* LPCOLLISIONEVENT;

class Collision
{
private:
    static Collision* __instance;

public:
    static Collision* GetInstance();

    void SweptAABB(
        float ml, float mt, float mr, float mb,
        float dx, float dy,
        float sl, float st, float sr, float sb,
        float& t, float& nx, float& ny);
    void Scan(
        GameObject* objSrc,
        float dx, float dy,
        vector<GameObject*>* coObjects,
        vector<LPCOLLISIONEVENT>& coEvents);
};