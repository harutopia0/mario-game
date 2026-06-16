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
    
    if (pipeHeight <= 3) {
        // Vẽ ống mặc định lún xuống đất
        float sinkAmount = (3 - pipeHeight) * 15.0f;
        if (defaultAni != NULL) defaultAni->Render(x, y - sinkAmount);
    } else {
        int numSupp = pipeHeight - 3;
        // Vẽ ống mặc định ở phần ngọn TRƯỚC (nằm dưới cùng về order z-index)
        if (defaultAni != NULL) defaultAni->Render(x, y + numSupp * 15.0f);
        
        // Vẽ các đốt thân bổ sung từ trên xuống dưới SAU
        // Dùng vòng lặp ngược để đốt dưới đè lên đốt trên 1 pixel
        for(int i = numSupp - 1; i >= 0; i--) {
            if (suppAni != NULL) suppAni->Render(x, y + i * 15.0f);
        }
    }
}