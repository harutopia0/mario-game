#pragma once
#include "Block.h"

// Lớp cha của các khối loại 2 (Brick xây thêm, Breakable, LuckyBlock) - Có thể bị phá hủy
class DynamicBlock : public Block {
public:
    DynamicBlock(float x, float y) : Block(x, y) {
        this->isStatic = true; // Dù có thể vỡ, nhưng bản chất vẫn nằm im cản đường
        this->layer = LAYER_BLOCKS;
    }
    virtual ~DynamicBlock() {}

    // Phương thức chung để gọi khi khối bị phá hủy (bởi FireBlast, SukunaSlash...)
    virtual void Break(bool dropItem = true) = 0;
};
