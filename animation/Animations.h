#pragma once
#include <unordered_map>
#include "Animation.h"

class Animations
{
private:
    static Animations* __instance;
    std::unordered_map<int, Animation*> animations;

public:
    static Animations* GetInstance();
    void Add(int id, Animation* ani);
    void Clear()
    {
        for (auto x : animations)
        {
            Animation* ani = x.second;
            delete ani;
        }
        animations.clear();
    }
    Animation* Get(int id);
};