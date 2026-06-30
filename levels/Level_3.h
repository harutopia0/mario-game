#pragma once
#include "Level.h"

class CLevel3 : public CLevel
{
public:
	CLevel3(int lv_id, int time) : CLevel(lv_id, time) {};
	void LoadLevel() override;
};