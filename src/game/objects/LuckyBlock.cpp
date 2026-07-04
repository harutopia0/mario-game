#include "game/objects/LuckyBlock.h"
#include "engine/audio/AudioManager.h"
#include "engine/graphics/Animations.h"
#include "game/items/Buff.h"
#include "game/scenes/GameManager.h"
#include "game/scenes/Map.h"
#include <cstdlib>

LuckyBlock::LuckyBlock(float x, float y, int activeAnimationId, int usedAnimationId)
    : DynamicBlock(x, y)
{
    this->activeAnimationId = activeAnimationId;
    this->usedAnimationId = usedAnimationId;
    Animation *anim = Animations::GetInstance()->Get(activeAnimationId);
    if (anim != NULL)
    {
        this->width = anim->GetWidth();
        this->height = anim->GetHeight();
    }
    else
    {
        this->width = 16;
        this->height = 16;
    }
    this->isHit = false;
    // this->isStatic is already set by DynamicBlock
}

void LuckyBlock::GetBoundingBox(float &left, float &top, float &right, float &bottom)
{
    left = x;
    top = y;
    right = x + width;
    bottom = y + height;
}

void LuckyBlock::Render()
{
    int aniId = isHit ? usedAnimationId : activeAnimationId;
    Animation *ani = Animations::GetInstance()->Get(aniId);
    if (ani != NULL)
        ani->Render(x, y);
}

void LuckyBlock::SpawnItem()
{
    int items[] = {301, 302, 303, 304};
    int randomItem = items[rand() % 4];

    Buff *buff = new Buff(x, y, randomItem);
    buff->StartSprouting(y);
    Map::GetInstance()->GetObjects().push_back(buff);
    Map::GetInstance()->AddObjectToGrid(buff);

    AudioManager::GetInstance()->PlaySFX("mushroom_sound_effect");
}

void LuckyBlock::Hit()
{
    if (!isHit)
    {
        isHit = true;
        SpawnItem();
        GameManager::GetInstance()->AddScore(100);
    }
}

void LuckyBlock::Break(bool dropItem)
{
    if (!isHit && dropItem)
    {
        SpawnItem();
    }
    this->Delete();
}