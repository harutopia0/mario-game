#pragma once
#include <Windows.h>
#include <vector>
#include "../render/Sprite.h"

class AnimationFrame
{
private:
    Sprite* sprite;
    DWORD time;

public:
    AnimationFrame(Sprite* sprite, int time) { this->sprite = sprite; this->time = time; }
    DWORD GetTime() { return time; }
    Sprite* GetSprite() { return sprite; }
};

class Animation
{
private:
    ULONGLONG lastFrameTime;
    int defaultTime;
    int currentFrame;
    std::vector<AnimationFrame*> frames;

public:
    Animation(int defaultTime = 100);
    void Add(int spriteId, DWORD time = 0);
    void Render(float x, float y);
};