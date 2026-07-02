#include "Pipe.h"
#include "../animation/Animations.h"

Pipe::Pipe(float x, float y, int pipeHeight, bool canEnter, float destX, float destY)
    : StaticBlock(x, y)
{
    this->pipeHeight = pipeHeight;
    this->width = 32.0f;
    
    // Sprite miệng ống cao 46px (~3 block), thân ống 15px mỗi đốt
    // Ống ngắn (<=3 block): chỉ có miệng ống, cao 46px
    // Ống dài (>3 block): miệng ống + (pipeHeight-3) đốt thân, mỗi đốt 15px
    if (pipeHeight <= 3) {
        this->height = 46.0f;
    } else {
        this->height = (pipeHeight - 3) * 15.0f + 46.0f;
    }
    
    this->canEnter = canEnter;
    this->destX = destX;
    this->destY = destY;
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
    
    // Ống spawn từ dưới lên:
    // y = đáy ống (sát mặt đất), Y tăng lên trên
    // Thân ống xếp từ đáy, miệng ống nằm trên cùng
    
    int numSupp = (pipeHeight > 3) ? (pipeHeight - 3) : 0;
    
    // 1. Vẽ các đốt thân ống từ đáy (y) lên trên
    for (int i = 0; i < numSupp; i++) {
        if (suppAni != NULL) {
            suppAni->Render(x, y + i * 15.0f);
        }
    }
    
    // 2. Vẽ miệng ống nối tiếp phía trên phần thân
    if (defaultAni != NULL) {
        defaultAni->Render(x, y + numSupp * 15.0f);
    }
}