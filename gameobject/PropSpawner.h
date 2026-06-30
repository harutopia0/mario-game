#pragma once
#include <vector>

class GameObject;

struct PropDef {
    int spriteId;
    float width;
    float height;
    float minSpace;
    int weight;
    float minGap;
    float maxGap;
};

class PropSpawner {
private:
    std::vector<PropDef> propDefs;
    int emptyWeight;
    float emptyMinGap;
    float emptyMaxGap;

public:
    PropSpawner();
    void AddProp(PropDef def);
    void SetEmptySpace(int weight, float minGap, float maxGap);
    
    std::vector<GameObject*> SpawnProps(int cols, const std::vector<std::vector<int>>& mapData, float groundY);
    std::vector<GameObject*> SpawnClouds(int cols, float minHeight, float maxHeight);
};
