#pragma once
#include <Windows.h>
#include <algorithm>
#include <vector>
using namespace std;

// Hệ thống phân lớp hiển thị (Z-Index)
// Số nhỏ vẽ trước (chìm dưới), số lớn vẽ sau (đè lên trên)
#define LAYER_BACKGROUND  0   // Mario lúc chui ống nước
#define LAYER_BLOCKS      1   // Gạch, Ống, Đất, Platform, Flag, LuckyBlock, Breakable
#define LAYER_ITEMS       2   // Nấm, Hoa, Buff
#define LAYER_ENEMIES     3   // Quái vật (Goomba, Koopa...)
#define LAYER_PLAYER      4   // Mario, Fireball

class GameObject
{
protected:
    float x, y;
    float vx, vy;
    int nx;
    bool isDeleted;
    int layer;

public:
    int GetLayer() const { return layer; }
    void SetLayer(int l) { layer = l; }

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