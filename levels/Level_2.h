#pragma once
#include "Level.h"

class CLevel2 : public CLevel
{
public:
	CLevel2 (int lv_id. int time) : CLevel (lv_id, time){}
	void LoadLevel() override; 
};