#include "Breakable.h"
#include "../animation/Animations.h"
#include "../gameplay/Map.h"
#include "BrickDebrisEffect.h"

Breakable::Breakable(float x, float y, int animationId) : DynamicBlock(x, y)
{
    this->animationId = animationId;
    Animation* anim = Animations::GetInstance()->Get(animationId);
    if (anim != NULL) {
        this->width = anim->GetWidth();
        this->height = anim->GetHeight();
    } else {
        this->width = 16;
        this->height = 16;
    }
    this->leftNeighbor = nullptr;
    this->rightNeighbor = nullptr;
    this->isDynamicRender = false;
    // this->isStatic is already set by DynamicBlock
}

void Breakable::SetRenderParams(bool top, bool floating, int aTop)
{
    this->isDynamicRender = true;
    this->isTop = top;
    this->isFloating = floating;
    this->animTop = aTop;
}

void Breakable::UpdateRenderLogic()
{
    if (!isDynamicRender) return;

    bool hasLeft = (leftNeighbor != nullptr);
    bool hasRight = (rightNeighbor != nullptr);

    if (animTop == 211) { // Grass Top
        if (isTop) {
            if (isFloating) {
                if (!hasLeft && !hasRight) animationId = 238; // Floating Isolated
                else if (!hasLeft) animationId = 236; // Floating Left
                else if (!hasRight) animationId = 237; // Floating Right
                else animationId = 235; // Floating Center
            } else {
                if (!hasLeft && !hasRight) animationId = 228;
                else if (!hasLeft) animationId = 226;
                else if (!hasRight) animationId = 227;
                else animationId = 225;
            }
        } else {
            if (!hasLeft && !hasRight) animationId = 232;
            else if (!hasLeft) animationId = 230;
            else if (!hasRight) animationId = 231;
            else animationId = 229;
        }
    }
}

void Breakable::OnNeighborBroken(Breakable* neighbor)
{
    if (neighbor == leftNeighbor) leftNeighbor = nullptr;
    if (neighbor == rightNeighbor) rightNeighbor = nullptr;
    UpdateRenderLogic();
}

void Breakable::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void Breakable::Render()
{
    Animation* ani = Animations::GetInstance()->Get(animationId);
    if (ani != NULL) ani->Render(x, y);
}

void Breakable::Break(bool dropItem)
{
    if (isDeleted) return;

    // Spawn VFX
    BrickDebrisEffect* debris = new BrickDebrisEffect(x, y, animationId);
    Map::GetInstance()->GetObjects().push_back(debris);
    Map::GetInstance()->AddObjectToGrid(debris);

    this->isDeleted = true;
    
    if (leftNeighbor) leftNeighbor->OnNeighborBroken(this);
    if (rightNeighbor) rightNeighbor->OnNeighborBroken(this);
    this->Delete();
}