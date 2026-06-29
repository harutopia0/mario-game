#include "Ground.h"
#include "GameObject.h"
#include "Pipe.h"
#include "Flower.h"

void CLevel4 :: LoadLevel()
{

	#pragma once
#include "Level.h"

class CLevel4 : public CLevel
{
public:
    CLevel4() : CLevel(4, 300, "Level_4.txt") {} 
    ~CLevel4 () {}
    void Update(DWORD dt) override; 
};
