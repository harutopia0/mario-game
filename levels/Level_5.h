#pragma once
#include "Level.h"

class CLevel5 : public CLevel
{
public:
	CLevel5(int lv_id, int time) : CLevel(lv_id, time) {};
	void LoadLevel() override;
};