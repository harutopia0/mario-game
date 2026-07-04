#include "game/items/PropSpawner.h"
#include "game/items/Prop.h"
#include <cstdlib>

PropSpawner::PropSpawner()
{
    emptyWeight = 0;
    emptyMinGap = 0.0f;
    emptyMaxGap = 0.0f;
}

void PropSpawner::AddProp(PropDef def)
{
    propDefs.push_back(def);
}

void PropSpawner::SetEmptySpace(int weight, float minGap, float maxGap)
{
    emptyWeight = weight;
    emptyMinGap = minGap;
    emptyMaxGap = maxGap;
}

std::vector<GameObject *> PropSpawner::SpawnProps(int cols, const std::vector<std::vector<int>> &mapData, float groundY)
{
    std::vector<GameObject *> spawnedProps;
    int c = 0;
    while (c < cols)
    {
        bool isGround = false;
        if (mapData.size() > 14)
        {
            isGround = (mapData[13][c] == 1 || mapData[14][c] == 1);
        }

        if (isGround)
        {
            int length = 0;
            while (c + length < cols && (mapData[13][c + length] == 1 || mapData[14][c + length] == 1))
            {
                length++;
            }

            float startX = c * 15.0f;
            float totalWidth = length * 15.0f;
            float currentX = startX + (rand() % 100 + 50); // Padding ngẫu nhiên ở đầu

            while (currentX <= startX + totalWidth)
            {
                float spaceLeft = (startX + totalWidth) - currentX;

                std::vector<PropDef> validProps;
                int totalWeight = emptyWeight;

                for (size_t i = 0; i < propDefs.size(); i++)
                {
                    if (spaceLeft >= propDefs[i].minSpace)
                    {
                        int c_prop = (int)(currentX / 15.0f);
                        if (c_prop >= cols)
                            c_prop = cols - 1;

                        int groundRow = 14;
                        if (mapData.size() > 13 && mapData[13][c_prop] == 1)
                            groundRow = 13;
                        if (mapData.size() > 12 && mapData[12][c_prop] == 1)
                            groundRow = 12;

                        float propGroundY = ((15 - groundRow - 1) * 15.0f) + 35.0f + 15.0f;

                        bool collision = false;
                        int c_start = (int)(currentX / 15.0f);
                        int c_end = (int)((currentX + propDefs[i].width) / 15.0f);
                        for (int cc = c_start; cc <= c_end && cc < cols; cc++)
                        {
                            for (int rr = 0; rr < groundRow; rr++)
                            {
                                if (mapData.size() > rr && mapData[rr].size() > cc && mapData[rr][cc] != 0)
                                {
                                    float cellYBottom = ((15 - rr - 1) * 15.0f) + 35.0f;
                                    float cellYTop = cellYBottom + 15.0f;
                                    float propYBottom = propGroundY;
                                    float propYTop = propGroundY + propDefs[i].height;

                                    if (propYBottom < cellYTop && propYTop > cellYBottom)
                                    {
                                        collision = true;
                                        break;
                                    }
                                }
                            }
                            if (collision)
                                break;
                        }

                        if (!collision)
                        {
                            validProps.push_back(propDefs[i]);
                            validProps.back().minSpace = propGroundY; // Tạm dùng minSpace để lưu propGroundY
                            totalWeight += propDefs[i].weight;
                        }
                    }
                }

                if (totalWeight <= 0)
                {
                    float gap = emptyMinGap;
                    if (emptyMaxGap > emptyMinGap)
                    {
                        gap += (rand() % (int)(emptyMaxGap - emptyMinGap + 1));
                    }
                    currentX += gap;
                    continue; // Bỏ qua nếu không có khoảng trống cho bất kỳ prop nào
                }

                int randomPick = rand() % totalWeight;
                int currentWeight = 0;
                bool picked = false;

                for (size_t i = 0; i < validProps.size(); i++)
                {
                    currentWeight += validProps[i].weight;
                    if (randomPick < currentWeight)
                    {
                        float finalGroundY = validProps[i].minSpace; // Lấy lại groundY đã tính
                        Prop *prop = new Prop(currentX, finalGroundY, validProps[i].spriteId, validProps[i].width, validProps[i].height);
                        spawnedProps.push_back(prop);

                        float gap = validProps[i].minGap;
                        if (validProps[i].maxGap > validProps[i].minGap)
                        {
                            gap += (rand() % (int)(validProps[i].maxGap - validProps[i].minGap + 1));
                        }
                        currentX += validProps[i].width + gap;
                        picked = true;
                        break;
                    }
                }

                if (!picked)
                {
                    float gap = emptyMinGap;
                    if (emptyMaxGap > emptyMinGap)
                    {
                        gap += (rand() % (int)(emptyMaxGap - emptyMinGap + 1));
                    }
                    currentX += gap;
                }
            }
            c += length;
        }
        else
        {
            c++;
        }
    }

    return spawnedProps;
}

std::vector<GameObject *> PropSpawner::SpawnClouds(int cols, float minHeight, float maxHeight)
{
    std::vector<GameObject *> spawnedClouds;
    float startX = 0;
    float totalWidth = cols * 15.0f;
    float currentX = startX + (rand() % 100 + 50);

    while (currentX <= totalWidth)
    {
        float spaceLeft = totalWidth - currentX;
        std::vector<PropDef> validProps;
        int totalWeight = emptyWeight;

        for (size_t i = 0; i < propDefs.size(); i++)
        {
            if (spaceLeft >= propDefs[i].minSpace)
            {
                validProps.push_back(propDefs[i]);
                totalWeight += propDefs[i].weight;
            }
        }

        if (totalWeight <= 0)
            break;

        int randomPick = rand() % totalWeight;
        int currentWeight = 0;
        bool picked = false;

        for (size_t i = 0; i < validProps.size(); i++)
        {
            currentWeight += validProps[i].weight;
            if (randomPick < currentWeight)
            {
                float y = minHeight;
                if (maxHeight > minHeight)
                {
                    y += (rand() % (int)(maxHeight - minHeight + 1));
                }

                Prop *prop = new Prop(currentX, y, validProps[i].spriteId, validProps[i].width, validProps[i].height);
                spawnedClouds.push_back(prop);

                float gap = validProps[i].minGap;
                if (validProps[i].maxGap > validProps[i].minGap)
                {
                    gap += (rand() % (int)(validProps[i].maxGap - validProps[i].minGap + 1));
                }
                currentX += validProps[i].width + gap;
                picked = true;
                break;
            }
        }

        if (!picked)
        {
            float gap = emptyMinGap;
            if (emptyMaxGap > emptyMinGap)
            {
                gap += (rand() % (int)(emptyMaxGap - emptyMinGap + 1));
            }
            currentX += gap;
        }
    }
    return spawnedClouds;
}
