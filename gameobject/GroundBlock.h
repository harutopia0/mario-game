#pragma once
#include "StaticBlock.h"

// Khối gạch nền dùng cho việc vẽ nền đất dưới cùng (Loại 1 - không phá được)
class GroundBlock : public StaticBlock {
private:
    float width;
    float height;
    int animationId;

public:
    GroundBlock(float x, float y, int animationId);

    virtual void Render();
    virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom);
};
