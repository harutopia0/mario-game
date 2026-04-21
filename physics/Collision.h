#pragma once

#include <vector>
#include "../gameplay/GameObject.h"

class CCollision
{
private:
    static CCollision* __instance;

public:
    static CCollision* GetInstance();

    // kiểm tra AABB đơn giản (giống hàm nền trong engine)
    void SweptAABB(
        float ml, float mt, float mr, float mb,
        float sl, float st, float sr, float sb
    );

    // check 2 object có va chạm không (wrapper giống SweptAABB style)
    bool CheckCollision(GameObject* src, GameObject* dest);

    // lấy danh sách collision (giống Scan style)
    void Scan(
        GameObject* src,
        std::vector<GameObject*>* objects,      
        std::vector<LPCOLLISIONEVENT>& events
    );
};