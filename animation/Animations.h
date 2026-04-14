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
    Animation* Get(int id);
};