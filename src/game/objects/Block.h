#pragma once
#include "engine/core/GameObject.h"

// Lớp cha của tất cả các khối trong game
class Block : public GameObject {
public:
    Block(float x, float y) : GameObject(x, y) {}
    virtual ~Block() {}
    
    // Trả về true nếu đây là khối chỉ va chạm 1 chiều (từ trên xuống)
    virtual bool IsOneWay() const { return false; }
};
