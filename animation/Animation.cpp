#include "Animation.h"
#include "../render/Sprites.h"

Animation::Animation(int defaultTime)
{
    this->defaultTime = defaultTime;
    lastFrameTime = -1;
    currentFrame = -1;
}

void Animation::Add(int spriteId, DWORD time)
{
    int t = time;
    if (time == 0) t = this->defaultTime;

    Sprite* sprite = Sprites::GetInstance()->Get(spriteId);
    AnimationFrame* frame = new AnimationFrame(sprite, t);
    frames.push_back(frame);
}

void Animation::Render(float x, float y)
{
    ULONGLONG now = GetTickCount64();
    if (currentFrame == -1)
    {
        currentFrame = 0;
        lastFrameTime = now;
    }
    else
    {
        DWORD t = frames[currentFrame]->GetTime();
        if (now - lastFrameTime > t)
        {
            currentFrame++;
            lastFrameTime = now;
            if (currentFrame == frames.size()) currentFrame = 0;
        }
    }

    frames[currentFrame]->GetSprite()->Draw(x, y);
}