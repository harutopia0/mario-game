#pragma once
#include "../core/GameObject.h"

// Lớp cha của tất cả các khối trong game
class Block : public GameObject {
public:
    Block(float x, float y) : GameObject(x, y) {}
    virtual ~Block() {}
};
