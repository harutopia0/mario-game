#include "PropSpawner.h"
#include "Prop.h"
#include <cstdlib>

PropSpawner::PropSpawner() {
    emptyWeight = 0;
    emptyMinGap = 0.0f;
    emptyMaxGap = 0.0f;
}

void PropSpawner::AddProp(PropDef def) {
    propDefs.push_back(def);
}

void PropSpawner::SetEmptySpace(int weight, float minGap, float maxGap) {
    emptyWeight = weight;
    emptyMinGap = minGap;
    emptyMaxGap = maxGap;
}

std::vector<GameObject*> PropSpawner::SpawnProps(int cols, const std::vector<std::vector<int>>& mapData, float groundY) {
    std::vector<GameObject*> spawnedProps;
    int c = 0;
    while (c < cols) {
        bool isGround = false;
        if (mapData.size() > 14) {
            isGround = (mapData[13][c] == 1 || mapData[14][c] == 1);
        }
        
        if (isGround) {
            int length = 0;
            while (c + length < cols && (mapData[13][c + length] == 1 || mapData[14][c + length] == 1)) {
                length++;
            }
            
            float startX = c * 15.0f;
            float totalWidth = length * 15.0f;
            float currentX = startX + (rand() % 100 + 50); // Padding ngẫu nhiên ở đầu
            
            while (currentX <= startX + totalWidth) {
                float spaceLeft = (startX + totalWidth) - currentX;
                
                std::vector<PropDef> validProps;
                int totalWeight = emptyWeight; 
                
                for (size_t i = 0; i < propDefs.size(); i++) {
                    if (spaceLeft >= propDefs[i].minSpace) {
                        validProps.push_back(propDefs[i]);
                        totalWeight += propDefs[i].weight;
                    }
                }
                
                if (totalWeight <= 0) {
                    break;
                }
                
                int randomPick = rand() % totalWeight;
                int currentWeight = 0;
                bool picked = false;
                
                for (size_t i = 0; i < validProps.size(); i++) {
                    currentWeight += validProps[i].weight;
                    if (randomPick < currentWeight) {
                        Prop* prop = new Prop(currentX, groundY, validProps[i].spriteId, validProps[i].width, validProps[i].height);
                        spawnedProps.push_back(prop);
                        
                        float gap = validProps[i].minGap;
                        if (validProps[i].maxGap > validProps[i].minGap) {
                            gap += (rand() % (int)(validProps[i].maxGap - validProps[i].minGap + 1));
                        }
                        currentX += validProps[i].width + gap;
                        picked = true;
                        break;
                    }
                }
                
                if (!picked) {
                    float gap = emptyMinGap;
                    if (emptyMaxGap > emptyMinGap) {
                        gap += (rand() % (int)(emptyMaxGap - emptyMinGap + 1));
                    }
                    currentX += gap;
                }
            }
            c += length;
        } else {
            c++;
        }
    }
    
    return spawnedProps;
}

std::vector<GameObject*> PropSpawner::SpawnClouds(int cols, float minHeight, float maxHeight) {
    std::vector<GameObject*> spawnedClouds;
    float startX = 0;
    float totalWidth = cols * 15.0f;
    float currentX = startX + (rand() % 100 + 50);
    
    while (currentX <= totalWidth) {
        float spaceLeft = totalWidth - currentX;
        std::vector<PropDef> validProps;
        int totalWeight = emptyWeight; 
        
        for (size_t i = 0; i < propDefs.size(); i++) {
            if (spaceLeft >= propDefs[i].minSpace) {
                validProps.push_back(propDefs[i]);
                totalWeight += propDefs[i].weight;
            }
        }
        
        if (totalWeight <= 0) break;
        
        int randomPick = rand() % totalWeight;
        int currentWeight = 0;
        bool picked = false;
        
        for (size_t i = 0; i < validProps.size(); i++) {
            currentWeight += validProps[i].weight;
            if (randomPick < currentWeight) {
                float y = minHeight;
                if (maxHeight > minHeight) {
                    y += (rand() % (int)(maxHeight - minHeight + 1));
                }
                
                Prop* prop = new Prop(currentX, y, validProps[i].spriteId, validProps[i].width, validProps[i].height);
                spawnedClouds.push_back(prop);
                
                float gap = validProps[i].minGap;
                if (validProps[i].maxGap > validProps[i].minGap) {
                    gap += (rand() % (int)(validProps[i].maxGap - validProps[i].minGap + 1));
                }
                currentX += validProps[i].width + gap;
                picked = true;
                break;
            }
        }
        
        if (!picked) {
            float gap = emptyMinGap;
            if (emptyMaxGap > emptyMinGap) {
                gap += (rand() % (int)(emptyMaxGap - emptyMinGap + 1));
            }
            currentX += gap;
        }
    }
    return spawnedClouds;
}
