#include "Ground.h"
#include "GameObject.h"
#include "Pipe.h"
#include "Flower.h"

void CLevel5 :: LoadLevel()
{

	#pragma once
#include "Level.h"

class CLevel5 : public CLevel
{
public:
    CLevel5() : CLevel(5, 300, "Level_5.txt") {} 
    ~CLevel5 () {}
    void Update(DWORD dt) override; 
};
