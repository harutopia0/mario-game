#include "Pipe.h"
#include "../animation/Animations.h"

Pipe::Pipe(float x, float y, int pipeHeight, bool canEnter, float destX, float destY)
    : StaticBlock(x, y)
{
    this->pipeHeight = pipeHeight;
    this->width = 32.0f;
    this->height = pipeHeight * 15.0f;
    
    this->canEnter = canEnter;
    this->destX = destX;
    this->destY = destY;
    // this->isStatic is already set by StaticBlock
    this->layer = LAYER_BACKGROUND;
}

void Pipe::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void Pipe::Render()
{
    Animation* defaultAni = Animations::GetInstance()->Get(204);
    Animation* suppAni = Animations::GetInstance()->Get(207);
    
    // Luôn vẽ phần đầu ống (miệng ống) ở trên cùng (y)
    if (defaultAni != NULL) {
        defaultAni->Render(x, y);
    }
    
    // Nếu ống dài hơn 3 block (chiều cao > 45px), vẽ thêm các phần thân bổ sung ở dưới
    if (pipeHeight > 3) {
        int numSupp = pipeHeight - 3;
        for (int i = 0; i < numSupp; i++) {
            // Phần miệng ống cao 45px (3 block 15px), nên thân bắt đầu từ y + 45
            // Mỗi đốt thân cao 15px
            if (suppAni != NULL) {
                suppAni->Render(x, y + 45.0f + i * 15.0f);
            }
        }
    }
}