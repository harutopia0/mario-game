#include "Level.h"
#include "Ground.h"
#include "GameObject.h"
#include "Pipe.h"
#include "Flower.h"

void CLevel3 :: LoadLevel()
{

	#pragma once
#include "Level.h"

class CLevel3 : public CLevel
{
public:
    CLevel3() : CLevel(3, 300, "Level_3.txt") {} 
    ~CLevel3 () {}
    void Update(DWORD dt) override; 
};
