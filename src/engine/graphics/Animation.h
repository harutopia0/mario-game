#pragma once
#include <Windows.h>
#include <vector>
#include "engine/graphics/Sprite.h"

class AnimationFrame
{
private:
    Sprite* sprite;
    DWORD time;

public:
    AnimationFrame(Sprite* sprite, int time) { this->sprite = sprite; this->time = time; }
    ~AnimationFrame() {}
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
    ~Animation()
    {
        for (int i = 0; i < frames.size(); i++) delete frames[i];
        frames.clear();
    }
    void Add(int spriteId, DWORD time = 0);
    void Render(float x, float y);
    void Render(float x, float y, int nx);
    void Render(float x, float y, int nx, int ny);
    void Render(float x, float y, float angle);
    void RenderScaled(float x, float y, float scaleX, float scaleY);
    void Render(float x, float y, D3DXCOLOR color);
    void Render(float x, float y, float drawWidth, float drawHeight, float alpha = 0.5f);
    int GetWidth();
    int GetHeight();
};