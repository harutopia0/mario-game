#pragma once
#include "game/objects/DynamicBlock.h"

class Breakable : public DynamicBlock
{
private:
    float width;
    float height;
    int animationId;

    bool isDynamicRender;
    bool isTop;
    bool isFloating;
    int animTop;

public:
    Breakable* leftNeighbor;
    Breakable* rightNeighbor;

    Breakable(float x, float y, int animationId);

    void SetRenderParams(bool top, bool floating, int aTop);
    void UpdateRenderLogic();
    void OnNeighborBroken(Breakable* neighbor);

    void GetBoundingBox(float& left, float& top, float& right, float& bottom) override;
    void Render() override;

    virtual void Break(bool dropItem = true);
};