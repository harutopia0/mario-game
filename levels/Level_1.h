#pragma once
#include "Level.h"

class CLevel1 : public CLevel
{
public: 
	CLevel1(int lv_id, int time) : CLevel(lv_id, time) {};
	void LoadLevel() override;
};