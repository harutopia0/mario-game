#pragma once
#include "Level.h"

class CLevel4 : public CLevel
{
public:
	CLevel4(int lv_id, int time) : CLevel(lv_id, time) {};
	void LoadLevel() override;
};