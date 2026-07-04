#pragma once
#include "game/objects/Block.h"

// Lớp cha của các khối loại 1 (Nền, Platform, Ống) - Không thể bị phá hủy
class StaticBlock : public Block
{
  public:
    StaticBlock(float x, float y) : Block(x, y)
    {
        this->isStatic = true;
        this->layer = LAYER_BLOCKS;
    }
    virtual ~StaticBlock()
    {
    }
};
