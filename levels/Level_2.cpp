#include "Level.h"
#include "Ground.h"
#include "GameObject.h"
#include "Pipe.h"
#include "Flower.h"

void CLevel2 :: LoadLevel()
{

	#pragma once
#include "Level.h"

class CLevel2 : public CLevel
{
public:
    CLevel2() : CLevel(2, 300, "Level_2.txt") {} 
    ~CLevel2    () {}
    void Update(DWORD dt) override; 
};
